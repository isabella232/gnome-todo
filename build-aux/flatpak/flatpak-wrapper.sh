#!/usr/bin/env bash

if [ "$1" = "--quit" -o "$1" = "-q" ]; then
    /app/bin/gnome-todo "$@"
    exit
fi

export GIO_USE_NETWORK_MONITOR=base
gsettings reset org.gnome.evolution-data-server network-monitor-gio-name
/app/libexec/evolution-source-registry &
sleep 2
/app/libexec/evolution-addressbook-factory -r &
/app/libexec/evolution-calendar-factory -r &
sleep 2
/app/bin/gnome-todo "$@"
