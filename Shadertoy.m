(* Shadertoy.m *)
BeginPackage["Shadertoy`"]

Begin["`Private`"]

$packageDir = DirectoryName[$InputFileName];
$programName = "shadertoy.bin"
$executable = FileNameJoin[{$packageDir, $programName}]; (* See Install[] documentation, Details section, for how it resolves directories into executables *)

Install[$executable]

End[] (* `Private` *)

EndPackage[]
