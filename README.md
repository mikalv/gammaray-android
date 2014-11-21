# GammaRay Extensions for Android
This repository is an educational (not production-ready)
class project to mount and log writes to a
remote block device on Android.
The logging is done with the
[GammaRay](https://github.com/cmusatyalab/gammaray) project,
which provides filesystem-level streams of block device
operations.

# Use Cases
## Device Management for Corporate Security Policies.
Corporate phones can use a network block device with LVM to mirror
Android's physical block devices as files on a remote server
to synchronize the device containing `/system` to a central
server when employees are on-site.

## Application Behavior Analysis for Malware Detection.
The amount of applications on the marketplace is approaching
1 million, and seemingly benign applications could
perform malicious actions on a user's device.
This use case would involve using nbd with LVM to
mirror the phone's block devices to a remote server
that has the capabilities of streaming the block writes in real time
when an application is performing.

Then, each application can be experimentally profiled and
paired with a sequence of hard disk reads and writes.
These sequences could reveal the nature of the application,
and perhaps a malicious application could somehow gain
privilege escalation and modify the contents of `/system`.

This technique of malware analysis by associating an
application with some stream of events is called *dynamic analysis*,
and the paper [Applying machine learning classifiers to dynamic
Android malware detection at scale][antimalware] further
discusses this approach.
[antimalware]: http://ieeexplore.ieee.org/xpls/abs_all.jsp?arnumber=6583806

## Filesystem Development Debugging.
Consider a systems-level developer modifying the Android system
and Linux kernel. Some change that she made to a complex internal
function call is causing kernel panics and thinks monitoring
the state of the device's storage leading up to the crash
will help reveal the bug.
Low level filesystem debugging can be done with nbd with LVM
to mirror and stream the hard disk writes to a remote server.

# GammaRay Features
GammaRay provides disk-based
[introspection](http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.11.8367)
for virtual machines as shown in the following diagram.
The block device writes are intercepted and Redis maintains the
streaming metadata.
The inference engine infers the filesystem operations that occured
in some time interval.

![](https://raw.githubusercontent.com/wenluhu/gammaray-android/master/img/GammaRay-Original.png)

# Extensions for Android
This repository uses [Network Block Device (nbd)](https://github.com/yoe/nbd)
on Android to connect to an export provided by GammaRay
as shown in the following diagram.
Writes to the nbd on Android are stored in Redis on the server
and can be introspected.

![](https://raw.githubusercontent.com/wenluhu/gammaray-android/master/img/GammaRay-Android.png)

# Demo
## Initialize the server.
Install and run `redis-server` and build the GammaRay binaries.
Run [demo/init-server.sh][init-server] to create the nbd export
of the partition on port `30004` through GammaRay's custom nbd queuer.
This will use Python to start an HTTP server on port `8000`
to view the contents of the block device.

[init-server]: https://github.com/wenluhu/gammaray-android/blob/master/demo/init-server.sh

## Connect the phone.
Next, follow the instructions in
[this blog post](http://bamos.github.io/2014/09/08/nbd-android/)
to connect to the server's nbd export on port `30004`.
Mount the partition to `/sdcard/DCIM/Camera`
and pictures taken from the device's camera will
be captured on the GammaRay server.
