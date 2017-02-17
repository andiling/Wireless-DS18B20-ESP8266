function Remove-StringLatinCharacters
{
    PARAM ([string]$String)
    [Text.Encoding]::ASCII.GetString([Text.Encoding]::GetEncoding("Cyrillic").GetBytes($String))
}

#--------------------------------------------------------------------------------------

#si un arg est present, c'est le fichier a traiter
if($args -and $args[0]) { $chemin=$args }
else
{
    #sinon on demande le chemin du dossier à traiter
    $chemin = @(Read-Host "File Path to convert")
}

$chemin=$chemin.trim().replace('"','');

#open files
[System.IO.FileStream] $fichierBinaire = [System.IO.File]::OpenRead($chemin);
[System.IO.FileStream] $fichierCode = [System.IO.File]::OpenWrite($chemin+".cpp");

#Prepare start of code and write it
$text="const PROGMEM bool gz = "
$text+= if($chemin.ToLower().EndsWith(".gz")){"true"}else{"false"}
$text+=";`r`nconst PROGMEM char "+ (Remove-StringLatinCharacters (Split-Path -Path $chemin -Leaf).Replace(' ','').Replace('.','')) + "[] = {";
$fichierCode.Write([System.Text.UTF8Encoding]::new().GetBytes($text),0,$text.Length);



$first=$true;

while($fichierBinaire.Position -ne $fichierBinaire.Length){

    #
    $text = if($first){""}else{","};
    $first=$false;
    $text+= "0x"+[System.BitConverter]::ToString($fichierBinaire.ReadByte());
    $fichierCode.Write([System.Text.UTF8Encoding]::new().GetBytes($text),0,$text.Length);
}

$text="};";
$fichierCode.Write([System.Text.UTF8Encoding]::new().GetBytes($text),0,$text.Length);

$fichierBinaire.Close();
$fichierCode.Close();
