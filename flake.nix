{
  description = "A collection of tools for emulating tracker responses.";

  inputs = rec {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    root-curated = {
      url = "github:umd-lhcb/root-curated";
      inputs.nixpkgs.follows = "nixpkgs";
      inputs.flake-utils.follows = "flake-utils";
    };
  };

  outputs = { self, nixpkgs, flake-utils, root-curated }:
    {
      overlay = import ./nix/overlay.nix;
    }
    //
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          overlays = [ self.overlay root-curated.overlay ];
        };
        python = pkgs.python3;
        pythonPackages = python.pkgs;
      in
      {
        devShell = pkgs.mkShell rec {
          name = "TrackerOnlyEmu-dev";
          buildInputs = with pythonPackages; [
            pkgs.clang-tools # For clang-format
            pkgs.root

            # Auto completion
            jedi

            # Linters
            flake8
            pylint

            # Python requirements (enough to get a virtualenv going).
            virtualenvwrapper
          ];

          shellHook = ''
            # Allow the use of wheels.
            SOURCE_DATE_EPOCH=$(date +%s)

            if test -d $HOME/build/python-venv; then
              VENV=$HOME/build/python-venv/${name}
            else
              VENV=./.virtualenv
            fi

            if test ! -d $VENV; then
              virtualenv $VENV
            fi
            source $VENV/bin/activate

            # allow for the environment to pick up packages installed with virtualenv
            export PYTHONPATH=$VENV/${python.sitePackages}/:$PYTHONPATH

            # fix libstdc++.so not found error
            export LD_LIBRARY_PATH=${pkgs.stdenv.cc.cc.lib}/lib:$LD_LIBRARY_PATH
          '';
        };
      });
}
