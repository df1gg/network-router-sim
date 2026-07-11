{
  pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/nixos-unstable.tar.gz") {
    config = { };
    overlays = [ ];
  },
}:
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    gdb
    pkg-config
  ];

  buildInputs = with pkgs; [ ];

  shellHook = ''
    echo "C development environment ready!"
    gcc --version
  '';
}
