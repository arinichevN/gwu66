#!/bin/bash

APP=gwu66
APP_DBG=`printf "%s_dbg" "$APP"`
INST_DIR=/usr/sbin
CONF_DIR=/etc/controller
CONF_DIR_APP=$CONF_DIR/$APP
PID_DIR=/var/run

SOCK=udp

MODE_DEBUG=-DMODE_DEBUG
MODE_FULL=-DMODE_FULL
#PLATFORM=-DPLATFORM_ANY
#PLATFORM=-DPLATFORM_A20
PLATFORM=-DPLATFORM_H3

NONE=-DNONEANDNOTHINE


function move_bin {
	([ -d $INST_DIR ] || mkdir $INST_DIR) && \
	cp $APP $INST_DIR/$APP && \
	chmod a+x $INST_DIR/$APP && \
	chmod og-w $INST_DIR/$APP && \
	echo "Your $APP executable file: $INST_DIR/$APP";
}

function move_bin_dbg {
	([ -d $INST_DIR ] || mkdir $INST_DIR) && \
	cp $APP_DBG $INST_DIR/$APP_DBG && \
	chmod a+x $INST_DIR/$APP_DBG && \
	chmod og-w $INST_DIR/$APP_DBG && \
	echo "Your $APP executable file for debugging: $INST_DIR/$APP_DBG";
}

function move_conf {
	([ -d $CONF_DIR ] || mkdir $CONF_DIR) && \
	([ -d $CONF_DIR_APP ] || mkdir $CONF_DIR_APP) && \
	cp  config.tsv $CONF_DIR_APP && \
	cp  device.tsv $CONF_DIR_APP && \
	chmod -R a+w $CONF_DIR_APP
	echo "Your $APP configuration files are here: $CONF_DIR_APP";
}

#your application will run on OS startup
function conf_autostart {
	cp -v starter_init /etc/init.d/$APP && \
	chmod 755 /etc/init.d/$APP && \
	update-rc.d -f $APP remove && \
	update-rc.d $APP defaults 30 && \
	echo "Autostart configured";
}

function build_lib {
	gcc $1 $PLATFORM -c app.c -D_REENTRANT -pthread && \
	gcc $1 $PLATFORM -c crc.c
	gcc $1 $PLATFORM -c gpio.c && \
	gcc $1 $PLATFORM -c 1w.c && \
	gcc $1 $PLATFORM -c timef.c && \
	gcc $1 $PLATFORM -c $SOCK.c && \
	gcc $1 $PLATFORM -c util.c && \
	gcc $1 $PLATFORM -c max6675.c && \
	
	cd acp && \
	gcc $1 $PLATFORM -c main.c && \
	cd ../ && \
	echo "library: making archive..." && \
	rm -f libpac.a
	ar -crv libpac.a 1w.o app.o crc.o gpio.o timef.o $SOCK.o util.o max6675.o acp/main.o && echo "library: done"
	rm -f *.o acp/*.o
}
#    1         2
#debug_mode bin_name
function build {
	cd lib && \
	build_lib $1 && \
	cd ../ 
	gcc -D_REENTRANT $1 $3 $PLATFORM main.c -o $2 -pthread -L./lib -lpac && echo "Application successfully compiled. Launch command: sudo ./"$2
}

function full {
	build $NONE $APP $MODE_FULL && \
	build $MODE_DEBUG $APP_DBG $MODE_FULL && \
	move_bin && move_bin_dbg && move_conf && conf_autostart
}
function full_nc {
	build $NONE $APP $MODE_FULL && \
	build $MODE_DEBUG $APP_DBG $MODE_FULL  && \
	move_bin && move_bin_dbg
}
function part_debug {
	build $MODE_DEBUG $APP_DBG $NONE
}
function uninstall {
	pkill -F $PID_DIR/$APP.pid --signal 9
	update-rc.d -f $APP remove
	rm -f $INST_DIR/$APP
	rm -f $INST_DIR/$APP_DBG
	rm -rf $CONF_DIR_APP
}
function uninstall_nc {
	pkill -F $PID_DIR/$APP.pid --signal 9
	rm -f $INST_DIR/$APP
	rm -f $INST_DIR/$APP_DBG
}
f=$1
${f}