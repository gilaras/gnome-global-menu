#! /bin/sh
libexec_dir=$2
cat > $1 <<EOF
[Xfce Panel]
Type=X-XFCE-PanelPlugin
Encoding=UTF-8
Name=Global Menu Plugin
Comment=Placeholder for GTK+ application menu in XFCE Panel
Icon=xfce4-settings
X-XFCE-Exec=${libexec_dir}/xfce-globalmenu-plugin
EOF
