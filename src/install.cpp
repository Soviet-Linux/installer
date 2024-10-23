#include "../include/install.hpp"
#include "../include/utils.hpp"

int execInChroot(const std::string& command) {
    std::string fullCommand = "systemd-nspawn -D /mnt --as-pid2 bash -c \"" + command + "\"";

    return std::system(fullCommand.c_str());
}

int setRootPassword(const std::string& newPassword) {
    std::string command = "echo 'root:" + newPassword + "' | chpasswd";

    return execInChroot(command);
}

int setupSystemd() {
    return std::system("systemd-firstboot --root=/mnt --setup-machine-id --root-shell=/bin/bash --force");
}

int partition_drive(const std::string& target_drive) {
    return std::system(("sgdisk -n 1:0:+512M -c 1:\"SOVIET-EFI\" -t 1:ef00 -n 2:0:0 -c 2:\"sovietlinux\" -t 2:8304 " + target_drive).c_str());
}

int create_boot(const std::string& target_drive) {
    return std::system(("mkfs.vfat -F 32 -n SOVIET-EFI " + target_drive + "1").c_str());
}

int create_root(const std::string& target_drive) {
    return std::system(("mkfs.btrfs -L sovietlinux " + target_drive + "2").c_str());
}

int mount_root() {
    const char* source = "LABEL=sovietlinux";
    const char* target = "/mnt";
    const char* filesystemtype = "btrfs";
    const char* options = "compress=zstd:3";

    if (mount(source, target, filesystemtype, 0, options) != 0) {
        std::cerr << "Failed to mount: " << std::strerror(errno) << std::endl;
        return -1;
    }

    std::cout << "Successfully mounted " << source << " at " << target
              << " with options: " << options << std::endl;

    return 0;
}

int mount_boot() {
    const char* source = "LABEL=SOVIET-EFI";
    const char* target = "/mnt/efi";
    const char* filesystemtype = "vfat";
    const char* options = "defaults";

    if (mount(source, target, filesystemtype, 0, options) != 0) {
        std::cerr << "Failed to mount: " << std::strerror(errno) << std::endl;
        return -1;
    }

    std::cout << "Successfully mounted " << source << " at " << target
              << " with options: " << options << std::endl;

    return 0;
}

void install_soviet(const std::string& target_drive,
                    const std::string& locale,
                    const std::string& keymap,
                    const std::string& rootPassword,
                    const std::string& hostname) {
    std::string kernel_version = getKernelVersion();
    std::string build_id = getVersionID("/mnt/etc/os-release");
    partition_drive(target_drive);
    create_boot(target_drive);
    create_root(target_drive);
    mount_root();
    mount_boot();
    copyFiles("/run/rootfsbase/", "/mnt");
    deleteFilesInDir("/mnt/efi/EFI/Linux/");
    setupSystemd();
    execInChroot("localectl set-keymap --no-convert " + keymap);
    execInChroot("localectl set-locale LANG=" + locale);
    setRootPassword(rootPassword);
    execInChroot("hostnamectl set-hostname " + hostname);
    execInChroot("systemd-machine-id-setup");
    execInChroot("gpg --import /lib/systemd/import-pubring.gpg");
    execInChroot("chattr +C /var/log/journal");
    execInChroot("dracut -H -I ' /usr/bin/nano ' --add-drivers ' vfat btrfs ' --strip /tmp/sov-initrd.img");
    execInChroot("/usr/lib/systemd/ukify build --linux=/usr/lib/modules/" + kernel_version + "/vmlinuz-soviet --initrd=/tmp/sov-initrd.img --uname=" + kernel_version + " --splash=/efi/logo-soviet-boot.bmp --cmdline=@/etc/kernel/cmdline --output=/efi/EFI/Linux/sovietlinux-" + build_id + "-initrd.efi");
    execInChroot("bootctl random-seed");
}
