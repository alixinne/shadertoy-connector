(* Shadertoy.m *)
BeginPackage["Shadertoy`"]

(* CCompilerDriver` is not included in BeginPackage to avoid keeping it in the $ContextPath *)
Needs["CCompilerDriver`"]

Begin["`Private`"]

$packageDir = DirectoryName[$InputFileName];
$programName = "shadertoy.bin"
$executable = FileNameJoin[{$packageDir, $programName}]; (* See Install[] documentation, Details section, for how it resolves directories into executables *)

(* This is the list of source files to be compiled. There may be multiple .cxx and .tm files,
   all of which will be linked together. See the CreateExecutable[] documentation,
   Details section, on how to include additional object files. See CreateObjectFile[]
   on how to create an object file without also linking it into an executable. *)
$sourceFiles = FileNameJoin[{$packageDir, "Sources", #}]& /@
	{"shadertoy.tm", "context.cpp", "host.cpp", "local.cpp", "remote.cpp", "shadertoy_mathematica.cpp"};

(* Just a colourful Print alternative *)
print[args___] := Print @@ (Style[#, Red]&) /@ {args}

(* Compile and install *)
If[
    Install[$executable] === $Failed
    ,
    print["Compiling Shadertoy executable..."];
    CreateExecutable[$sourceFiles, $programName,
     "TargetDirectory" -> FileNameJoin[{$executable, $SystemID}],
     "IncludeDirectories" -> { "/usr/include", "/usr/local/include", "/usr/include/jsoncpp" },
     "Libraries" -> { "curl", "jsoncpp", "glfw", "shadertoy0", "boost_filesystem", "boost_system", "GLEW", "GL" },
     (* the following two options are here to ease debugging *)
     "ShellCommandFunction" -> Print, (* see compilation command *)
     "ShellOutputFunction" -> Print (* see compiler messages *)
    ];
    If[
        Install[$executable] === $Failed
        ,
        print["Failed to compile or run Shadertoy executable. \[SadSmiley]"],
        print["Compiling successful! \[HappySmiley]"]
    ]
]

End[] (* `Private` *)

EndPackage[]
