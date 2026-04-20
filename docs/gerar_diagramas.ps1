$ErrorActionPreference = "Stop"

$repo = Resolve-Path (Join-Path $PSScriptRoot "..")
$doxygen = "C:\Program Files\doxygen\bin\doxygen.exe"
$dot = "C:\Program Files\Graphviz\bin\dot.exe"

if (!(Test-Path -LiteralPath $doxygen)) {
    $doxygen = "doxygen"
}

if (!(Test-Path -LiteralPath $dot)) {
    $dot = "dot"
}

Push-Location $repo
try {
    & $doxygen Doxyfile
    & $dot -Tpdf docs\diagrama_classes_graphviz.dot -o docs\diagrama_classes_graphviz.pdf
    & $dot -Tsvg docs\diagrama_classes_graphviz.dot -o docs\diagrama_classes_graphviz.svg
}
finally {
    Pop-Location
}
