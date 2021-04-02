# macrodevice Lua API

## ``macrodevice.arg``
A table containing the arguments that are being passed to Lua using the ``--arg`` or ``-a`` commandline option.

## ``macrodevice.close()``
Requests closing all opened devices.

## ``macrodevice.close(id)``
id: integer

Requests closing the device with the given id.

## ``macrodevice.drop_root(uid, gid)``
uid: integer, gid: integer

Drops root permissions by setting the real and effective user and group ids to uid and gid.

Returns 1 in case of failure, otherwise 0

## ``macrodevice.drop_root(username)``
username: string

Drops root permissions by setting the real and effective user and group ids to uid and gid of the user with the given username.

Returns 1 in case of failure, otherwise 0

## ``macrodevice.open(backend, settings, event_handler)``
backend: string, settings: table, event_handler: function

Opens the device specified by the settings table using the given backend. event_handler is the callback function that gets called for every incoming event.

Returns the unique id of the opened device or nil in case of failure.

## ``macrodevice.open(settings, event_handler)``
settings: table, event_handler: function

Opens the device specified by the settings table using the backend from the "backend" field in the settings table. event_handler is the callback function that gets called for every incoming event.

Returns the unique id of the opened device or nil in case of failure.

## ``macrodevice.version``
A string containing the version of macrodevice.