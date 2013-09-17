#!/usr/bin/php -q
<?php
//define(BLACK_AND_WHITE, true);
$tmpDATA = "./tmpDATA.dat";
$tmpPLOT = "./tmpPLOT.plot";
$TPL = 'set terminal fig
set key right bottom
set size 1.0, 1.0
set output \'{OUT_FILE}\'
set xlabel "number of VMs"  offset character 0, 0, 0 font "" norotate
set title ""  offset character 0, 0, 0 font "" norotate
set ylabel "Time to allocate VMs (s)" offset character 0, 0, 0 font "gh,5" textcolor lt -1 rotate by 90
plot {PLOT_CMD}
';

if ($argc < 3) {
	echo "usage : ".$argv[0]. "simFolder( i.e CLOUD_X/simFolder) outFile\n";
	exit(-1);
}



$simFolder = $argv[1];
$outFile = $argv[2];
$nb_insts = 4;


include_once('db_wldout.php');
include_once('workloadTools.php');




    include_once('db_wldout.php');
    
    $cloudFolder = array("opennebula", "openstack");
    $cloud = array(0=> "OpenNebula", 1=>"OpenStack");
    
   //nb vms
    $lambdas = array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10 , 11, 12, 13, 14, 15 /*, 16, 17, 18*/);
    
    
    $wantedCloud = array(true, true);
	$wantedCloudOrder = array(0, 1);
	
	for($inst=1; $inst <= $nb_insts; $inst++) {
	
		for($l=0; $l < count($lambdas); $l++) {
			
			for($i=0; $i<count($cloud); $i++) {
				if ($wantedCloud[$i]){
					$fileOut[$i][$l][$inst] = $cloudFolder[$i]."/".$simFolder."/".$lambdas[$l]."_i$inst.txt";
					echo $fileOut[$i][$l][$inst]."\n";
				}
			}	
		
		}
	
	}
	
	for($inst=1; $inst <= $nb_insts; $inst++) {
		for($a = 0; $a < count($wantedCloud); $a++) {
			if ($wantedCloud[$a]) {
				for($l=0; $l < count($lambdas); $l++) {
					$db[$a][$l][$inst] = dbcsv_open($fileOut[$a][$l][$inst]);
				}
			}
		}
     }
    
    
    
    	for($i=0; $i<count($cloud); $i++) {
    		
    		if ($wantedCloud[$i]){	
				for($l=0; $l < count($lambdas); $l++) {
				
					$sum = 0.0;
					for ($inst = 1; $inst <= $nb_insts; $inst++) {
						$sum += $db[$i][$l][$inst]['data'][0][0];
					}
					
					
					$time[$i][$l] = $sum / $nb_insts;
					
					//exit(0);
					
					//echo "$i $l => ".$time[$i][$l]."\n";	
				}
    		}
   		 }
  	  
    
	
	
	
    $offset = 2;
	$OFFSET=array();
    $fDATA = fopen($tmpDATA, "w+");
	fwrite($fDATA,"Cloud");
	for ($a = 0; $a < count($wantedCloud); $a++) {
		if ($wantedCloud[$a]) {
			fwrite($fDATA, "\t".$cloud[$a]);
			$OFFSET[$a] = $offset;
			$offset++;
		}
	}
	fwrite($fDATA, "\n");
	
	for ($s = 0; $s < count($lambdas); $s++) {
    	fwrite($fDATA, $lambdas[$s]);
		for ($a = 0; $a < count($wantedCloud); $a++) {
			if ($wantedCloud[$a]) {

				$alloc = $time[$a][$s];
				
				//$val = $makespan[$a][$s];
    			fwrite($fDATA, "\t".$alloc);
			}
		}
    	fwrite($fDATA, "\n");
	}
    fclose($fDATA);

    $plotCmd = "";
	for ($a = 0; $a < count($wantedCloudOrder); $a++) {
		if ($wantedCloud[$wantedCloudOrder[$a]]) {
			if (strlen($plotCmd) == 0)
				$plotCmd .= '[1:'.($lambdas[count($lambdas) - 1]).']\''.$tmpDATA.'\' using 1:'.($OFFSET[$wantedCloudOrder[$a]]).' t "'.$cloud[$wantedCloudOrder[$a]].'"';
			else
				$plotCmd .= ', \'\' u 1:'.($OFFSET[$wantedCloudOrder[$a]]).' t "'.($cloud[$wantedCloudOrder[$a]]).'"';

				$plotCmd .= " with linespoints pointtype ".($a+1);
		}
		
	}
	
	

    $fPLOT = fopen($tmpPLOT, "w+");
	
	
		
    $buff = str_replace(array('{OUT_FILE}', '{PLOT_CMD}'), array($outFile, $plotCmd), $TPL);
    fwrite($fPLOT, $buff);
    fclose($fPLOT);
    system('gnuplot tmpPLOT.plot');

?>
