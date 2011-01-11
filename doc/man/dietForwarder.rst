=============
dietForwarder
=============

:Author: haikel.guemar@sysfera.com
:Date:   2011-01-10
:Copyright: DIET developers
:License: GPLv3
:Version: 0.1
:Manual section: 1

NAME
====

dietForwarder - Tools for creating and maintaining ssh tunnels between DIET 
components in complex topologies

SYNOPSYS
========

  dietForwarder [options] ...

DESCRIPTION
===========

DietForwarder helps simplifying the maintenance of ssh tunnels between DIET 
components, thus improving DIET scalability and configuration in complex 
network topologies.

Before starting a DIET forwarder, you must:

* launch omniNames on the local and remotes hosts.

* launch the remote peer only defining its name and network configuration.

* launch local peer and give him remote peer's name, ssh connection informations, remote port to use and pass -C option to create the ssh tunnel.

[Remark: forwarders must be launched before the DIET hierarchy]

OPTIONS
=======

**--name** [name]
  String identifying the forwarder

**--peer-name** [name]
  String identifying its peer on the other network

**--ssh-host** [host]
  Host hosting the ssh tunnel

**--ssh-login** [login]
  Login used to establish the ssh connection (default: current user login).

**--ssh-key** [/path/to/ssh/key]
  Path to the ssh key (the private one !) used to establish the ssh connection 
  (default: $HOME/.ssh/id_rsa).

**--remote-port** [port]
  Port listening on the ssh host.

**--remote-host** [host]
  Host to which the connection is made by the tunnel (corresponds to ssh options -L and -R).

**--nb-retry** [nb]
  Number of times that the local forwarder will try to bind itself to the 
  remote forwarder (default: 3).

**--peer-ior** [IOR]
  Pass remote forwarder's IOR. By default, the local forwarder will retrive its peer IOR.

**--net-config** [path/to/configuration/file]
  Path to configuration file.

**-C** 
  Create the tunnel from this forwarder.


CONFIGURATION FILE
==================

You can pass a configuration file to dietForwarder instead of using command line options
through the --net-config option. Configuration file lists several rules describing 
networks reachable using this forwarder.

There's two category of rules:

**accept rules** 
  describe which networks are accessible through the forwarder.

**reject rules**
  describe which networks are not accessible through the forwarder.


A rule always starts by either **accept:** or **reject:** immediately 
followed by a regular expression (Posix) describing host concerned by the rule.
Rules are evaluated in the following order: **accept** then **reject**.
For instance:

  accept:.*
  reject:localhost

This fragment means that the forwarder will accept connections to every hosts 
but localhost.


EXAMPLE
=======

Here's a simple configuration:

* We have two domains: *net1* and *net2*, forwarders will be launched on hosts *fwd.net1* and *fwd.net2*. 

* There's no link between hosts *fwd.net1* and *fwd.net2* but user may access *fwd.net2* from *fwd.net1* using a ssh connection.

* We'll name *fwd.net1* forwarder Fwd1 and *fwd.net2* fowarder Fwd2.

* One SeD lives in *fwd.net2* while the rest of the DIET hierarchy lives on the *net1* domain.

**Command line for launchind Fwd1**

::

  fwd.net1$ dietForwarder --name Fwd1 --peer-name Fwd2 \
    --ssh-host fwd.net2 --ssh-login dietUser \
    --ssh-key id rsa net2 --remote-port 50000 \
    --net-config net1.cfg -C

**Command line to launch Fwd2**

::

  fwd.net2$ dietForwarder --name Fwd2 --net-config net2.cfg


**Configuration file for Fwd1**

In this example, the forwarders Fwd1 accepts only the connections to fwd.net2.

::

  accept:fwd.net2


**Configuration file for Fwd2**

In this example, the forwarders Fwd2 accepts all the connections except those which are
for the localhost.

::

  accept:.*
  reject:localhost


RATIONALE
=========

DIET uses CORBA as its communication layer. While it's a flexible and robust middleware, 
it remains hard deploying DIET on heterogeneous networks that are not reachable except 
through ssh tunnels. DIET forwarders help administrator configuring their grid without 
manually set-up ssh tunnels which arguably is neither simple nor scalable.
DIET forwarders make it very easy configuring such topologies.

LICENSE AND COPYRIGHT
=====================
    
Copyright
---------    
(C)2011, GRAAL, INRIA Rhone-Alpes, 46 allee d'Italie, 69364 Lyon
cedex 07, France all right reserved <diet-dev@ens-lyon.fr>

License
-------
This program is free software: you can redistribute it and/or mod‐
ify it under the terms of the GNU General Public License as pub‐
lished by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version. This program is
distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Pub- lic Li‐
cense for more details. You should have received a copy of the GNU
General Public License along with this program. If not, see
<http://www.gnu.org/licenses/>.

AUTHORS
=======
GRAAL
INRIA Rhone-Alpes
46 allee d'Italie 69364 Lyon cedex 07, FRANCE
Email: <diet-dev@ens-lyon.fr>
WWW: http://graal.ens-lyon.fr/DIET

SEE ALSO
========
omniNames(1), DIETAgent(1)

BUGS
====

On some systems, forwarder rules won't work unless you use 
IP addresses instead of hostnames 
