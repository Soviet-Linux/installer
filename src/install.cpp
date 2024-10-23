#include "../include/install.hpp"
#include "../include/utils.hpp"

#include <array>
#include <memory>



int execInChroot(const std::string& command) {
    std::cout << "Executing in chroot: " << command << std::endl;
    std::string fullCommand = "chroot /mnt bash -c \"" + command + "\"";

    std::array<char, 128> buffer;
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(fullCommand.c_str(), "r"), pclose);
    if (!pipe) {
        std::cerr << "Failed to open pipe for command: " << fullCommand << std::endl;
        std::exit(1);
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    int exitCode = WEXITSTATUS(pclose(pipe.release()));

    if (exitCode != 0) {
        std::cerr << "Command '" << command << "' failed with exit code: " << exitCode << std::endl;
        std::cerr << "Output: " << result << std::endl;  // Print command output only if there's an error
        std::exit(exitCode);
    }

    std::cout << "Command '" << command << "' executed successfully." << std::endl;
    return exitCode;
}

int setRootPassword(const std::string& newPassword) {
    std::cout << "Setting root password." << std::endl;
    std::string command = "echo 'root:sovietlinux' | chpasswd -e && echo 'root:" + newPassword + "' | chpasswd";
    return execInChroot(command);
}

int setupSystemd() {
    std::cout << "Setting up systemd." << std::endl;
    int result = std::system("systemd-firstboot --root=/mnt --setup-machine-id --root-shell=/bin/bash --force");

    if (result != 0) {
        std::cerr << "systemd setup failed with exit code: " << result << std::endl;
    }

    return result;
}

int partition_drive(const std::string& target_drive) {
    std::cout << "Partitioning drive: " << target_drive << std::endl;
    int result = std::system(("sgdisk -n 1:0:+512M -c 1:\"SOVIET-EFI\" -t 1:ef00 -n 2:0:0 -c 2:\"sovietlinux\" -t 2:8304 /dev/" + target_drive).c_str());

    if (result != 0) {
        std::cerr << "Partitioning drive failed with exit code: " << result << std::endl;
    }

    return result;
}

int create_boot(const std::string& target_drive) {
    std::cout << "Creating boot partition on drive: " << target_drive << std::endl;
    int result = std::system(("mkfs.vfat -F 32 -n SOVIET-EFI /dev/" + target_drive + "1").c_str());

    if (result != 0) {
        std::cerr << "Creating boot partition failed with exit code: " << result << std::endl;
    }

    return result;
}

int create_root(const std::string& target_drive) {
    std::cout << "Creating root partition on drive: " << target_drive << std::endl;
    int result = std::system(("mkfs.btrfs -L sovietlinux /dev/" + target_drive + "2").c_str());

    if (result != 0) {
        std::cerr << "Creating root partition failed with exit code: " << result << std::endl;
    }

    return result;
}

int mount_root(std::string target) {
    std::cout << "Mounting root partition: /dev/" << target << "2" << std::endl;
    std::string source_str = "/dev/" + target + "2";
    const char* source = source_str.c_str();
    const char* mount_point = "/mnt";
    const char* filesystemtype = "btrfs";
    const char* options = "compress=zstd:3";

    if (mount(source, mount_point, filesystemtype, 0, options) != 0) {
        std::cerr << "Failed to mount root partition: " << std::strerror(errno) << std::endl;
        return -1;
    }

    std::cout << "Successfully mounted root partition." << std::endl;
    return 0;
}

int mount_boot(std::string target) {
    std::cout << "Mounting boot partition: /dev/" << target << "1" << std::endl;
    std::string source_str = "/dev/" + target + "1";
    const char* source = source_str.c_str();
    const char* mount_point = "/mnt/efi";
    const char* filesystemtype = "vfat";

    if (mount(source, mount_point, filesystemtype, 0, 0) != 0) {
        std::cerr << "Failed to mount boot partition: " << std::strerror(errno) << std::endl;
        return -1;
    }

    std::cout << "Successfully mounted boot partition." << std::endl;
    return 0;
}

void install_soviet(const std::string& target_drive,
                    const std::string& locale,
                    const std::string& keymap,
                    const std::string& rootPassword,
                    const std::string& hostname) {
    std::cout << "Starting Soviet Linux installation." << std::endl;
    std::string kernel_version = getKernelVersion();
    std::string build_id = getVersionID("/mnt/etc/os-release");

    if (partition_drive(target_drive) != 0) return;
    if (create_boot(target_drive) != 0) return;
    if (create_root(target_drive) != 0) return;
    if (mount_root(target_drive) != 0) return;
    system("mkdir /mnt/efi");
    if (mount_boot(target_drive) != 0) return;

    std::cout << "Copying root filesystem." << std::endl;
    if (std::system("cp -Rv /run/rootfsbase/* /mnt") != 0) {
        std::cerr << "Failed to copy root filesystem." << std::endl;
        return;
    }
    system("ln -s /proc /mnt/proc");
    system("ln -s /sys /mnt/sys");
    system("ln -s /dev /mnt/dev");
    system("ln -s /run /mnt/run");

    deleteFilesInDir("/mnt/efi/EFI/Linux/");

    if (setupSystemd() != 0) return;

    if (execInChroot("echo 'KEYMAP=" + keymap + "' > /etc/vconsole.conf") != 0) return;
    //if (execInChroot("echo 'LANG="+ locale + "' > /etc/locale.conf && locale-gen") != 0) return;
    if (system(("echo 'root:" + rootPassword + "' | chpasswd -P /mnt").c_str()) != 0) return;
    if (execInChroot("echo "+ hostname +" > /etc/hostname") != 0) return;
    if (execInChroot("systemd-machine-id-setup") != 0) return;
    //if (execInChroot("gpg --import /lib/systemd/import-pubring.gpg") != 0) return;
    if (execInChroot("chattr +C /var/log/journal") != 0) return;

    if (execInChroot("dracut -H -I ' /usr/bin/nano ' --add-drivers ' vfat btrfs ' --strip /tmp/sov-initrd.img") != 0) return;
    if (execInChroot("/usr/lib/systemd/ukify build --linux=/usr/lib/modules/" + kernel_version + "/vmlinuz-soviet --initrd=/tmp/sov-initrd.img --uname=" + kernel_version + " --splash=/efi/logo-soviet-boot.bmp --cmdline=@/etc/kernel/cmdline --output=/efi/EFI/Linux/sovietlinux-" + build_id + "-initrd.efi") != 0) return;

    execInChroot("bootctl install");
    std::cout << "Soviet Linux installation completed." << std::endl;
}
