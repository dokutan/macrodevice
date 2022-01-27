;; Example config for macrodevice-lua
(print (.. "macrodevice version " macrodevice.version))

(fn input_handler [event]
  "Whenever an input event occurs, this function gets called.
  As an argument this function takes a table which describes the event.
  This table always contains strings."
  ;; Print the event
  (print (table.unpack event))
  ;; execute commands
  (match event
    [:EV_KEY key :1] (match key
                       :KEY_F1 (os.execute "mpc toggle &")
                       :KEY_F1 (os.execute "mpc prev &")
                       :KEY_F1 (os.execute "mpc next &")
                       :KEY_ESC :quit)))

;; The settings table specifies the device, and other settings.
;; The available setting keys depend on the backend, look at documentation/backends.md
(let [settings {:backend :libevdev
                :eventfile :/dev/input/by-id/usb-SONiX_USB_DEVICE-event-kbd
                :grab true
                :numbers false}]
  ;; open the device(s)
  (macrodevice.open settings input_handler))
