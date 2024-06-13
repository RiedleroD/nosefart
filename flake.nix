{
  description = "An over-engineered Flake for Nosefart";

  # Nixpkgs / NixOS version to use.
  inputs.nixpkgs.url = "nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      lastModifiedDate =
        self.lastModifiedDate or self.lastModified or "19700101";
      version = builtins.substring 0 8 lastModifiedDate;
      supportedSystems =
        [ "x86_64-linux" "x86_64-darwin" "aarch64-linux" "aarch64-darwin" ];
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;
      nixpkgsFor = forAllSystems (system:
        import nixpkgs {
          inherit system;
          overlays = [ self.overlay ];
        });
    in {
      overlay = final: prev: { inherit (self.packages) nosefart; };

      # Provide some binary packages for selected system types.
      packages = forAllSystems (system:
        let pkgs = nixpkgsFor.${system};
        in {
          nosefart = pkgs.callPackage ({ stdenv, SDL2 }:
            stdenv.mkDerivation {
              pname = "nosefart";
              inherit version;

              src = ./.;

              makeFlags = [ "PREFIX=$(out)" ];

              nativeBuildInputs = [ SDL2 ];
            }) { };
        });

      defaultPackage = forAllSystems (system: self.packages.${system}.nosefart);

      nixosModules.nosefart = { pkgs, ... }: {
        nixpkgs.overlays = [ self.overlay ];

        environment.systemPackages = [ pkgs.nosefart ];
      };

    };
}
