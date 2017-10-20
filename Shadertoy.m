(* Shadertoy.m *)
BeginPackage["Shadertoy`"]

Begin["`Private`"]

(* Find the binary location *)
$programName = "shadertoy.bin" <> If[StringContainsQ[$SystemID, "Windows"], ".exe", ""];
$executable = FileNameJoin[{
	DirectoryName[$InputFileName],
	"shadertoy.bin",
	$SystemID,
	$programName
}];

(* Terminate existing connection *)
If[Head[$currentLink] == LinkObject, Uninstall[$currentLink]];

(* Create new connection *)
$currentLink = Install[$executable];

End[] (* `Private` *)

EndPackage[]
