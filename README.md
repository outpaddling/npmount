# npmount
Non-privileged mount/unmount UI

## Purpose

NPMount is a command-line tool to provide limited mount and unmount
capabilities to non-privileged users, such as mounting and unmounting
external media including optical discs and thumb drives.

## Uses

NPMount can be used directly from the command-line or as a
platform-independent backend for GUI applications such as file managers.

It is more convenient than other methods because it is fully portable,
and never requires the user to enter a password.

## Security

NPmount is safer than sudo, since it never allows the execution of
arbitrary code under escalated privileges.  Only mount, umount, and
configuration update commands are ever run as root.

## Configuration file

The configuration file is simple.  It indicates which mount points
can be controlled by members of a given group.  The default configurtion
shown below indicates that members of the operator group can mount
and unmount directories under /media, which commonly contains mount points
for USB sticks and optical discs.

```
# Group     Mount-points
operator    /media/*
```

Mounting also requires that the mount point be listed in /etc/fstab
or equivalent.  The Unix mount command is issued with a mount point alone.

Unmounting does not require a listing in /etc/fstab, since the Unix
umount command needs only a mount point or device.

Most mounts of external media are performed by an automount service,
anyway, so use of npmount to mount media should not often be necessary.

## Usage

Using the configuration above as an example, members of the operator
group can run the following without authenticating:

npmount umount /media/cd0

If /media/cd0 is configured in /etc/fstab or equivalent, then members
of the operator group can also run

npmount mount /media/cd0

