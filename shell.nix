{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    # Инструменты сборки
    cmake
    ninja
    pkg-config
    gcc
    gdb

    # Qt6 библиотеки
    qt6.qtbase      # Включает Core, Gui, Widgets
    qt6.wrapQtAppsHook

    # Qt Creator из nixpkgs
    qtcreator
  ];

  shellHook = ''
    # Явно гарантируем, что CMake найдет все модули Qt6
    export CMAKE_PREFIX_PATH="${pkgs.qt6.qtbase}:$CMAKE_PREFIX_PATH"
    export QT_PLUGIN_PATH="${pkgs.qt6.qtbase}/${pkgs.qt6.qtbase.qtPluginPrefix}"
    export QML2_IMPORT_PATH="${pkgs.qt6.qtbase}/${pkgs.qt6.qtbase.qtQmlPrefix}"
  '';
}
