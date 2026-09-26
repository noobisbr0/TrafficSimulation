{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    cmake
    ninja
    pkg-config
    gcc
    gdb

    qt6.qtbase
    qt6.wrapQtAppsHook

    qtcreator
  ];

  shellHook = ''
    export CMAKE_PREFIX_PATH="${pkgs.qt6.qtbase}:$CMAKE_PREFIX_PATH"
    export QT_PLUGIN_PATH="${pkgs.qt6.qtbase}/${pkgs.qt6.qtbase.qtPluginPrefix}"
    export QML2_IMPORT_PATH="${pkgs.qt6.qtbase}/${pkgs.qt6.qtbase.qtQmlPrefix}"
  '';
}
