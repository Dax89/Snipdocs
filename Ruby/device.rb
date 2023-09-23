#! /bin/env ruby

class UDevEvent
  def initialize(type, event, path)
    @type = type
    @event = event
    @path = path
    @detail = {}
  end

  def device?
    type = @detail.fetch :DEVTYPE, ''
    type.end_with?('_device')
  end

  def update_detail(key, value)
    @detail[key.to_sym] = value
  end

  attr_reader :type, :event, :path, :detail
end

class UDevMonitor
  def initialize
    @device = nil
    @monitor = IO.popen 'udevadm monitor --property --udev'
  end

  def close
    @monitor.close
  end

  def run
    @monitor.each_line do |line|
      if @device.nil?
        m = /UDEV  \[[0-9.]+\] (add|remove) \s+ ([^ ]+) \(([^)]+)\)/.match line
        @device = UDevEvent.new(m[3], m[1], m[2]) if m && m[3] == 'usb'
      elsif !line.chomp.empty?
        m = /([A-Z_]+)=(.+)/.match(line)
        @device.update_detail(m[1], m[2]) if m
      else
        yield @device
        @device = nil
      end
    end
  end
end

# rubocop:disable Style/GlobalVars
$devices = {}
monitor = UDevMonitor.new

monitor.run do |d|
  if d.device?
    id = d.detail[:DEVPATH]

    case d.event
    when 'add'
      $devices[id] = d
    when 'remove'
      $devices.delete(id)
    else
      puts "Unknown event: '#{d.event}'"
    end

    puts $devices.values.inspect
  end
end

monitor.close
# rubocop:enable Style/GlobalVars
