#!/bin/env ruby
# rubocop:disable Style/GlobalVars

require 'json'
require 'dbus'

bus = DBus.session_bus
$bluez = bus['org.bluez']

# class AgentManager < DBus::Object
#   dbus_interface 'org.bluez.AgentManager1' do
#     dbus_method :RequestPinCode, 'in device:o' do |device|
#       puts 'Pin Code Requested'
#       puts device.inspect
#     end
#   end
# end
#
# service = bus.request_service 'org.gigi.RofiAgent'
# agent = AgentManager.new '/org/gigi/rofi_bluetooth_agent'
# service.export agent
#
# main = DBus::Main.new
# main << bus
# main.run

def rssi_to_icon(rssi)
  return 'device' if rssi.nil?

  if rssi == -100
    'network-wireless-signal-none-symbolic'
  elsif rssi >= -99 && rssi < -75
    'network-wireless-signal-weak-symbolic'
  elsif rssi >= -75 && rssi < -50
    'network-wireless-signal-ok-symbolic'
  elsif rssi >= -50 && rssi < -25
    'network-wireless-signal-good-symbolic'
  else
    'network-wireless-signal-excellent-symbolic'
  end
end

def read_adapters
  deviceobject = $bluez['/org/bluez']
  deviceobject.introspect

  deviceobject.subnodes.map do |x|
    adapter = $bluez.object File.join(deviceobject.path, x)
    adapter = adapter['org.bluez.Adapter1']
    { name: adapter['Name'], address: adapter['Address'], dbus_adapter: adapter, id: x }
  end
end

CONFIG_PATH = File.join(ENV['XDG_CONFIG_HOME'], 'rofi_bluetooth')
$adapters = read_adapters

def find_selected_adapter
  return $adapters[0] unless File.exist?(CONFIG_PATH)

  address = File.read(CONFIG_PATH).chomp
  $adapters.find { |x| x[:address] == address } || $adapters[0]
end

def read_devices
  root = $bluez['/']
  root.introspect

  objects = root['org.freedesktop.DBus.ObjectManager'].GetManagedObjects
                                                      .reject { |_, x| x['org.bluez.Device1'].nil? }

  objects.each_pair { |k, v| objects[k] = v['org.bluez.Device1'] }
  objects
end

def separator
  '\0nonselectable\x1ftrue'
end

def powered?(adapter)
  adapter[:dbus_adapter]['Powered']
end

def scanning?(adapter)
  adapter[:dbus_adapter]['Discovering']
end

def pairable?(adapter)
  adapter[:dbus_adapter]['Pairable']
end

def discoverable?(adapter)
  adapter[:dbus_adapter]['Discoverable']
end

def execute_rofi(payload, activeidx = nil)
  roficommand = "echo -e \"#{payload.join('\n')}\" | rofi -dmenu -p \"Bluetooth\""
  roficommand += " -a #{activeidx}" unless activeidx.nil?

  r = `#{roficommand}`.chomp
  r unless r.empty?
end

def unblock_rfkill(adapter)
  rfkill = JSON.parse(`rfkill -J`)['rfkilldevices']
  dev = rfkill.find { |x| x['device'] == adapter[:id] }
  return if dev.nil?

  system("rfkill unblock #{dev['id']} > /dev/null") if dev['soft'] == 'blocked' || dev['hard'] == 'blocked'
end

def create_boolean(label, condition, enabled: true)
  r = "#{condition ? '' : ''} #{label}\\0"
  r += '\x1fnonselectable\x1ftrue' unless enabled
  r
end

def do_scan(adapter)
  adapter[:dbus_adapter].StartDiscovery
  system("dunstify #{adapter[:name]} Scanning... -i bluetooth -a bluetooth > /dev/null")
  sleep 5
end

def show_device_manager(device, path)
  lines = [
    "#{device['Name']}\\0nonselectable\\x1ftrue\\x1ficon\\x1f#{rssi_to_icon(device['RSSI'])}",
    'Back\\0icon\\x1fback',
    separator
  ]

  lines.push device['Connected'] ? 'Disconnect' : 'Connect'
  lines.push device['Paired'] ? 'Unpair' : 'Pair'
  lines.push device['Blocked'] ? 'Unblock' : 'Block'

  choice = execute_rofi lines
  return if choice.nil?

  if choice == 'Back'
    show_main
    return
  end

  deviceobject = $bluez[path]
  deviceobject.introspect
  deviceinterface = deviceobject['org.bluez.Device1']

  case choice
  when 'Disconnect'
    deviceinterface.Disconnect
  when 'Connect'
    deviceinterface.Connect
  when 'Unpair'
    deviceinterface.CancelPairing
  when 'Pair'
    deviceinterface.Pair
  when 'Unblock'
    deviceinterface['Blocked'] = false
  when 'Block'
    deviceinterface['Blocked'] = true
  end

  show_device_manager device, path
end

def show_main
  adapter = find_selected_adapter
  defaultidx = nil

  adapters = $adapters.each_with_index.map do |x, i|
    default = x[:address] == adapter[:address]
    defaultidx = i if default
    "#{x[:address]} - #{x[:name]}\\0nonselectable\\x1f#{default}\\x1ficon\\x1fbluetooth"
  end

  founddevices = read_devices
  devices = []

  if scanning? adapter
    devices.push separator
    devices.push 'Update\0icon\x1freload'

    founddevices.each_pair do |_, v|
      d = "#{v['Name']} - #{v['Address']}"
      d += ' 󰂱' if v['Connected'] || v['Paired']
      d += ' 󰣐' if v['Trusted']
      d += ' ' if v['Blocked']
      d += "\\0icon\\x1f#{rssi_to_icon v['RSSI']}"
      devices.push d
    end
  end

  options = [separator, create_boolean('Powered', powered?(adapter))]

  if powered? adapter
    options.push create_boolean('Scanning', scanning?(adapter))
    options.push create_boolean('Pairable', pairable?(adapter))
    options.push create_boolean('Discoverable', discoverable?(adapter))
  end

  choice = execute_rofi adapters + devices + options, defaultidx
  return if choice.nil?

  if (adaptermatch = /^([0-9A-Z:]+) - .+$/.match(choice))
    File.write(CONFIG_PATH, adaptermatch[1])
  elsif choice.end_with? 'Powered'
    unblock_rfkill adapter[:id] unless powered? adapter
    adapter[:dbus_adapter]['Powered'] = !powered?(adapter)
  elsif choice.end_with? 'Update'
    adapter[:dbus_adapter].StopDiscovery
    do_scan adapter
  elsif choice.end_with? 'Scanning'
    if scanning? adapter
      adapter[:dbus_adapter].StopDiscovery
    else
      do_scan adapter
    end
  elsif choice.end_with? 'Pairable'
    adapter[:dbus_adapter]['Pairable'] = !pairable?(adapter)
  elsif choice.end_with? 'Discoverable'
    adapter[:dbus_adapter]['Discoverable'] = !discoverable?(adapter)
  elsif (devicematch = /^(.+) - ([0-9A-Z:]+)$/.match(choice))
    pair = founddevices.find { |_, v| v['Address'] == devicematch[2] }
    show_device_manager pair[1], pair[0] unless pair.nil?
    return
  end

  show_main
end

show_main

# rubocop:enable Style/GlobalVars
