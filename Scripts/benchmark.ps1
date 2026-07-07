param([int]$Iterations = 100000)
& "$PSScriptRoot\nxbuild.ps1" benchmark -Iterations $Iterations
