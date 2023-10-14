#!/bin/env ruby
# rubocop:disable Style/GlobalVars

require 'json'
require 'dbus'

DEFAULT_WIDTH  = 400
DEFAULT_HEIGHT = 200

bus = DBus.system_bus
$bluez = bus['org.bluez']

def read_devices
  deviceobject = $bluez['/org/bluez']
  deviceobject.introspect

  deviceobject.subnodes.map do |x|
    device = $bluez.object File.join(deviceobject.path, x)
    adapter = device['org.bluez.Adapter1']
    { name: adapter['Name'], address: adapter['Address'], adapter: adapter, id: x }
  end
end

CONFIG_PATH = File.join(ENV['XDG_CONFIG_HOME'], 'rofi_bluetooth')
$devices = read_devices

def find_selected_device
  return $devices[0] unless File.exist?(CONFIG_PATH)

  address = File.read(CONFIG_PATH).chomp
  $devices.find { |x| x[:address] == address }
end

def execute_tanto(payload)
  r = `echo -e '#{payload.to_json}' | tanto --stdin`.chomp
  return nil if r.empty?

  JSON.parse r
end

def unblock_rfkill(deviceid)
  rfkill = JSON.parse(`rfkill -J`)['rfkilldevices']
  dev = rfkill.find { |x| x['device'] == deviceid }
  return if dev.nil?

  system("rfkill unblock #{dev['id']} > /dev/null") if dev['soft'] == 'blocked' || dev['hard'] == 'blocked'
end

def show_device_manager(device)
  payload = {
    type: 'window',
    font: 'monospace',
    title: device[:name],
    width: DEFAULT_WIDTH,
    height: DEFAULT_HEIGHT * 2,
    fixed: true,

    body: {
      type: 'column',

      items: [
        { type: 'text', text: 'Devices' },
        { id: 'lvfound', type: 'list', stretch: true },
        { id: 'cbpower', type: 'check', text: 'Powered', checked: device[:adapter]['Powered'] },
        { id: 'cbpair', type: 'check', text: 'Pariable', checked: device[:adapter]['Pairable'] },
        { id: 'cbdiscover', type: 'check', text: 'Discoverable', checked: device[:adapter]['Discoverable'] },

        {
          type: 'row',

          items: [
            { id: 'btnback', type: 'button', text: '󰜱 Back' },
            { type: 'space' },
            { id: 'btnscan', type: 'button', text: device[:adapter]['Discovering'] ? 'Stop Scan' : '󰂰 Scan' }
          ]
        }
      ]
    }
  }

  r = execute_tanto payload
  return if r.nil?

  puts r.inspect
end

def show_devices
  default_device = find_selected_device

  payload = {
    type: 'window',
    title: 'Bluetooth',
    font: 'monospace',
    width: DEFAULT_WIDTH,
    height: DEFAULT_HEIGHT,
    fixed: true,

    body: {
      type: 'column',

      items: [
        { type: 'text', text: 'Available Adapters' },

        {
          id: 'lvadapters',
          type: 'list',

          items: $devices.map do |x|
            default = x[:address] == default_device[:address]
            { id: x[:address], text: default ? "󰂯 #{x[:name]} [DEFAULT]" : "󰂯 #{x[:name]}", selected: default }
          end
        }
      ]
    }
  }

  r = execute_tanto payload
  show_device_manager $devices[r['detail']['index']] unless r.nil?
end

show_devices

# rubocop:enable Style/GlobalVars
