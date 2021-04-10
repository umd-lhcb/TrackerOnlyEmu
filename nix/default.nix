{ stdenv
, buildPythonPackage
, numpy
, scikit-learn
, root
}:

# FIXME: We require uproot 4 but it's not in nixpkgs yet.

buildPythonPackage rec {
  pname = "TrackerOnlyEmu";
  version = "0.1.2";

  src = builtins.path { path = ./..; name = pname; };

  propagatedBuildInputs = [
    numpy
    scikit-learn
    root
  ];

  doCheck = false;
}
