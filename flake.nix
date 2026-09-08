{
  description = "A hash array mapped trie implementation in C";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "hamt";
          version = "0.1.0";

          src = ./.;

          buildInputs = [ ];

          buildPhase = ''
            runHook preBuild
            mkdir -p _build
            gcc -std=c11 -Iinclude -c src/hamt.c -o _build/hamt.o
            gcc -std=c11 -Iinclude main.c _build/hamt.o -o _build/main
            runHook postBuild
          '';

          installPhase = ''
            runHook preInstall
            mkdir -p $out/bin
            install -Dm755 _build/main $out/bin/hamt
            runHook postInstall
          '';
        };

        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            gcc
          ];
        };
      });
}
