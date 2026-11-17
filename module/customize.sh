#!/system/bin/sh
ui_print "- AETHER Serenity 0.9.2 WebUI"
if [ "$(getprop ro.product.cpu.abi)" != "arm64-v8a" ]; then
  abort "! AETHER supports arm64-v8a only."
fi
if [ ! -f "$MODPATH/system/bin/aether-supervisor" ]; then
  abort "! Missing aether-supervisor ARM64 binary."
fi
if [ ! -f "$MODPATH/webroot/index.html" ]; then
  abort "! Missing webroot/index.html."
fi
# Do not recursively chmod webroot: KernelSU manages its permissions and SELinux context.
set_perm "$MODPATH/service.sh" 0 0 0755
set_perm "$MODPATH/uninstall.sh" 0 0 0755
set_perm "$MODPATH/action.sh" 0 0 0755
set_perm "$MODPATH/system/bin/aether-supervisor" 0 0 0755
if [ ! -x "$MODPATH/system/bin/aether-supervisor" ]; then
  abort "! aether-supervisor is not executable after permission setup."
fi
