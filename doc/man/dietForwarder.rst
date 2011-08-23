=============
dietForwarder
=============

-------------------------------------------------------
DIET tool to manage ssh tunnels between DIET components
-------------------------------------------------------

:Author: haikel.guemar@sysfera.com
:Date:   2011-01-10
:Copyright: DIET developers
:License: GPLv3
:Version: 0.1
:Manual section: 1

NAME
====

dietForwarder - Tool for creating DIET forwarders. Forwarders are special
components that serve as proxies and manage ssh tunnels between DIET components in complex networks.

SYNOPSYS
========

  dietForwarder [options] ...

DESCRIPTION
===========

Deploying DIET on heterogeneous networks that are not reachable by means other than ssh tunnels
is a delicate task. DIET forwarders are special DIET components that serve as proxies between DIET components
by creating ssh tunnels.

dietForwarder is a command to instantiate DIET forwarders.
Forwarders simplify the maintenance of ssh tunnels between DIET components, thus improving DIET's scalability and ease of configuration in complex
network topologies.

Before starting a DIET forwarder, you must:

* launch omniNames on the local and remote hosts;

* launch the remote peer, specifying its name and network configuration only;

* launch the local peer and give it the remote peer's name, the ssh connection's details, the remote port to use and the 'pass -C' option to create the ssh tunnel.

[Remark: forwarders must be launched before the DIET hierarchy.]

OPTIONS
=======

**--name** [name]
  String identifying the forwarder.

**--peer-name** [name]
  String identifying its peer on the other network.

**--ssh-host** [host]
  Host hosting the ssh tunnel.

**--ssh-login** [login]
  Login used to establish the ssh connection (default: current user login).

**--ssh-key** [/path/to/ssh/key]
  Path to the ssh key (the private one!) used to establish the ssh connection
  (default: $HOME/.ssh/id_rsa).

**--remote-port** [port]
  Port the ssh host is listening on.

**--remote-host** [host]
  Host to which the connection is made by the tunnel (corresponds to ssh options -L and -R).

**--nb-retry** [nb]
  Number of times that the local forwarder will try to bind itself to the
  remote forwarder (default: 3).

**--peer-ior** [IOR]
  Passes the remote forwarder's IOR. By default, the local forwarder will retrieve its peer's IOR.

**--tunnel-wait** [seconds]
  Set the number of seconds to wait before considering that the tunnel
  has been created.


EXAMPLE
=======

Here is a simple configuration example:

* We have two domains: *net1* and *net2*. Forwarders will be launched on hosts *fwd.net1* and *fwd.net2*.

* There's no link between hosts *fwd.net1* and *fwd.net2* but users may access *fwd.net2* from *fwd.net1* using a ssh connection.

* Let's call the *fwd.net1* forwarder Fwd1 and the *fwd.net2* forwarder Fwd2.

* One SeD lives in *fwd.net2* while the rest of the DIET hierarchy lives on the *net1* domain.

**Command line for launching Fwd1**

::

  fwd.net1$ dietForwarder --name Fwd1 --peer-name Fwd2 \
    --ssh-host fwd.net2 --ssh-login dietUser \
    --ssh-key id rsa net2 --remote-port 50000

**Command line to launch Fwd2**

::

  fwd.net2$ dietForwarder --name Fwd2



RATIONALE
=========

DIET uses CORBA as its communication layer. While it is a flexible and robust middleware,
it remains hard to deploy DIET on heterogeneous networks that are not reachable except
through ssh tunnels. DIET forwarders help the administrator to configure their grid without
manually setting-up ssh tunnels, which is arguably neither simple nor scalable.
DIET forwarders make it very easy to configure networks with such topologies.

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
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
for more details. You should have received a copy of the GNU
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

On some systems, forwarder rules will not work unless you use
IP addresses instead of hostnames.
