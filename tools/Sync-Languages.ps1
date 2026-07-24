[CmdletBinding()]
param(
	[string]$RepositoryRoot
)

$ErrorActionPreference = 'Stop'

trap {
	Write-Error $_
	exit 1
}

if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
	$RepositoryRoot = Split-Path -Parent $PSScriptRoot
}

$languageDirectory = Join-Path $RepositoryRoot 'stage\AddonSpawner\Languages'
$englishFile = Join-Path $languageDirectory 'en-US.xml'
$fallbackFile = Join-Path $RepositoryRoot 'GTAVAddonLoader\Language.cpp'

function ConvertFrom-CppString([string]$value) {
	return [regex]::Replace($value, '\\(["\\tnr])', {
		param($match)
		switch ($match.Groups[1].Value) {
			'"' { '"' }
			'\' { '\' }
			't' { "`t" }
			'n' { "`n" }
			'r' { "`r" }
		}
	})
}

function Get-FallbackStrings([string]$path) {
	if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
		throw "Fallback source file was not found: $path"
	}
	$source = [System.IO.File]::ReadAllText($path)
	$mapMatch = [regex]::Match($source, 'const TStringMap sEnglishStrings = \{(?<entries>.*?)\n\};', [Text.RegularExpressions.RegexOptions]::Singleline)
	if (-not $mapMatch.Success) {
		throw "Could not locate sEnglishStrings in $path."
	}

	$strings = [ordered]@{}
	$entryPattern = '\{\s*"(?<key>(?:\\.|[^"\\])*)"\s*,\s*"(?<value>(?:\\.|[^"\\])*)"\s*\}'
	foreach ($entry in [regex]::Matches($mapMatch.Groups['entries'].Value, $entryPattern)) {
		$key = ConvertFrom-CppString $entry.Groups['key'].Value
		if ($strings.Contains($key)) {
			throw "Duplicate fallback key '$key' in $path."
		}
		$strings[$key] = ConvertFrom-CppString $entry.Groups['value'].Value
	}

	if ($strings.Count -eq 0) {
		throw "No fallback strings were parsed from $path."
	}
	return $strings
}

function Get-LanguageDocument([string]$path) {
	$document = New-Object System.Xml.XmlDocument
	$document.PreserveWhitespace = $false
	$document.Load($path)
	if ($null -eq $document.DocumentElement -or $document.DocumentElement.LocalName -ne 'language') {
		throw "$path does not contain a language root element."
	}
	return $document
}

function Get-LanguageEntries([System.Xml.XmlDocument]$document, [string]$path) {
	$entries = [ordered]@{}
	foreach ($node in $document.SelectNodes('/language/text')) {
		$key = $node.GetAttribute('key')
		if ([string]::IsNullOrWhiteSpace($key)) {
			throw "$path contains a text element without a key."
		}
		if ($entries.Contains($key)) {
			throw "$path contains duplicate key '$key'."
		}
		$entries[$key] = $node
	}
	return $entries
}

function Save-LanguageDocument([System.Xml.XmlDocument]$document, [string]$path) {
	$settings = New-Object System.Xml.XmlWriterSettings
	$settings.Indent = $true
	$settings.IndentChars = '  '
	$settings.Encoding = New-Object System.Text.UTF8Encoding($false)
	$writer = [System.Xml.XmlWriter]::Create($path, $settings)
	try {
		$document.Save($writer)
	}
	finally {
		$writer.Dispose()
	}
}

$fallbackStrings = Get-FallbackStrings $fallbackFile
$englishDocument = Get-LanguageDocument $englishFile

$englishNodes = Get-LanguageEntries $englishDocument $englishFile
$englishStrings = [ordered]@{}
foreach ($key in $fallbackStrings.Keys) {
	$englishStrings[$key] = $fallbackStrings[$key]
}

$englishNeedsUpdate = $false
if ($englishNodes.Count -ne $englishStrings.Count) {
	$englishNeedsUpdate = $true
}
else {
	foreach ($key in $englishStrings.Keys) {
		if (-not $englishNodes.Contains($key) -or $englishNodes[$key].InnerText -ne $englishStrings[$key]) {
			$englishNeedsUpdate = $true
			break
		}
	}
}

if ($englishNeedsUpdate) {
	foreach ($node in @($englishDocument.SelectNodes('/language/text'))) {
		$englishDocument.DocumentElement.RemoveChild($node) | Out-Null
	}
	foreach ($key in $englishStrings.Keys) {
		$node = $englishDocument.CreateElement('text')
		$node.SetAttribute('key', $key)
		$node.InnerText = $englishStrings[$key]
		$englishDocument.DocumentElement.AppendChild($node) | Out-Null
	}
	Save-LanguageDocument $englishDocument $englishFile
	Write-Host "Synchronized $([System.IO.Path]::GetFileName($englishFile))."
}

$languageFiles = Get-ChildItem -LiteralPath $languageDirectory -Filter '*.xml' -File | Where-Object { $_.FullName -ne (Get-Item -LiteralPath $englishFile).FullName }
foreach ($languageFile in $languageFiles) {
	$document = Get-LanguageDocument $languageFile.FullName
	$nodes = Get-LanguageEntries $document $languageFile.FullName
	$changed = $false

	foreach ($key in $englishStrings.Keys) {
		if (-not $nodes.Contains($key)) {
			$node = $document.CreateElement('text')
			$node.SetAttribute('key', $key)
			$node.InnerText = $englishStrings[$key]
			$document.DocumentElement.AppendChild($node) | Out-Null
			$nodes[$key] = $node
			$changed = $true
			continue
		}

	}

	foreach ($key in @($nodes.Keys)) {
		if (-not $englishStrings.Contains($key)) {
			$document.DocumentElement.RemoveChild($nodes[$key]) | Out-Null
			$changed = $true
		}
	}

	if ($changed) {
		Save-LanguageDocument $document $languageFile.FullName
		Write-Host "Synchronized $($languageFile.Name)."
	}
}
Write-Host 'Language packs are synchronized and en-US.xml matches the internal fallback strings.'
