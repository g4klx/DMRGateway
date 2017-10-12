#! /bin/bash
###############################################################################
#
# XLXHostsupdate.sh
#
# Copyright (C) 2016 by Tony Corbett G0WFV
# Copyright (C) 2017 by Jonathan Naylor G4KLX
#
# Adapted to YSFHosts by Paul Nannery KC2VRJ on 6/28/2016 with all crdeit 
# to G0WFV for the orignal script.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
###############################################################################
#
# On a Linux based system, such as a Raspberry Pi, this script will perform all  
# the steps required to maintain the XLXHosts.txt (or similar) file for you.
#
# It is designed to run from crontab and will download the XLXHosts from the 
# master XLX DMR Master database and optionally keep a backup of previously
# created files for you.
#
# It will also prune the number of backup files according to a value specified
# by you in the configuration below.
#
# To install in root's crontab use the command ...
#
#     sudo crontab -e
#
# ... and add the following line to the bottom of the file ...
#
#     0  0  *  *  *  /path/to/script/XLXHostsupdate.sh 1>/dev/null 2>&1
#
# ... where /path/to/script/ should be replaced by the path to this script.
#
###############################################################################
#
#                              CONFIGURATION
#
# Full path to XLXHosts
XLXHOSTS=/path/to/XLXHosts.txt

# How many XLXHosts files do you want backed up (0 = do not keep backups) 
XLXHOSTSFILEBACKUP=1

###############################################################################
#
# Do not edit below here
#
###############################################################################

# Check we are root
if [ "$(id -u)" != "0" ]; then
	echo "This script must be run as root" 1>&2
	exit 1
fi

# Create backup of old file
if [ ${XLXHOSTSFILEBACKUP} -ne 0 ]; then
	cp ${XLXHOSTS} ${XLXHOSTS}.$(date +%d%m%y)
fi

# Prune backups
BACKUPCOUNT=$(ls ${XLXHOSTS}.* | wc -l)
BACKUPSTODELETE=$(expr ${BACKUPCOUNT} - ${XLXHOSTSFILEBACKUP})

if [ ${BACKUPCOUNT} -gt ${XLXHOSTSFILEBACKUP} ]; then
	for f in $(ls -tr ${XLXHOSTS}.* | head -${BACKUPSTODELETE}); do
		rm -f $f
	done
fi

# Generate XLXHosts.txt file
curl http://xlxapi.rlx.lu/api.php?do=GetXLXDMRMaster | awk '
BEGIN {
	print "# The format of this file is:"
	print "# XLX Number;host;default"
}
/^XLX/ {
	reflector=4004
	if ($1 == "XLX313")
		reflector=4001
	if ($1 == "XLX950")
		reflector=4005
	printf "%s;%s;%d\n", substr($1,4), substr($2,1,length($2)-1), reflector
}' > ${XLXHOSTS}

exit 0
