cmd_/home/notgurev/se3-os-lab2/kernel/modules.order := {   echo /home/notgurev/se3-os-lab2/kernel/kmod.ko; :; } | awk '!x[$$0]++' - > /home/notgurev/se3-os-lab2/kernel/modules.order
