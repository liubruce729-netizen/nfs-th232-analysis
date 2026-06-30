Notes:
Calimero et GRU sont identique 
pour avoir un 

Les commandes
GRUC  localhost:8061  GRU INPUT FILE /space/legeard/run/runmuvigene.dat
ou 
GRUC  localhost:8061  CALIMERO INPUT FILE /space/legeard/run/runmuvigene.dat
sont identiques, le mot GRU et CALIMERO donne un traitement identique pour tout les fonctions sauf pour 
GRUC  localhost:8061  CALIMERO STOP 1
qui est l'equivalent de GRUC  localhost:8061  CALIMERO ENDCALIM 1
et GRUC  localhost:8061  GRU STOP 1 fait réélement un stop!


A partir de DAS ou GECO
Dans le fichier Common.java de DAS l'exe qui est lancé est 
/home/acqexp/Linux-i686/Calimero   qui est un lien vers un script en/home/acqexp/Linux-x86_64-el6/  que voici 
------------------------------------------------
killall Calimero.exe
source /home/global/root/rootenv.csh
source /home/acqexp/GRU/GRUenv.csh
$GRUDIR/bin/Calimero.exe 8061
-------------------------------------------------
Calimero.exe etant le meme executable que GRUCore il suffit de faire un lien.
Il faut garder ce lien car pour tuer Calimero ,il est nécessaire de connaitre le nom Calimero à ne pas confondre avec GRUCore qui pourrait etre utilisé en meme temps.

