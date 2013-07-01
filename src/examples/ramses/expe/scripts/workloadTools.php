<?php

include_once('db_wldout.php');

$setup=array();
$accelerator=array();


function getIndexTask($db, $name) {
	global $accelerator, $setup;
	
	if ($setup[$name] === true) {
		return $accelerator[$name];
	}
	
	
	for($i= 0; $i < count($db['data']); $i++) {
		if ($db['data'][$i][0] == $name) {
			$setup[$name] = true;
			$accelerator[$name] = $i;
			return $i;
		}
	}
	
	return -1;
}

function getTaskNumber($taskName){
	$matches = array();
	preg_match("/[0-9]+$/", $taskName, $matches);

	return $matches[0] + 0;
}


function getNBReqProcsStdIn($db_in, $index) {
	$req_procs = $db_in['data'][$index][5] + 0;
	
	return $req_procs;
}

function getNBAllocProcsOut($db_out, $index) {
	$allocProcs = $db_out['data'][$index][5] + 0;
	
	return $allocProcs;
}

function isMolded($db_in, $db_out, $indexOut) {
	$taskName = $db_out['data'][$indexOut][0];
	$taskNumber = getTaskNumber($taskName);
	
	$indexIn = $taskNumber - 1;
	
	$reqProcs = getNBReqProcsStdIn($db_in, $indexIn);
	$allocProcs = getNBAllocProcsOut($db_out, $indexOut);
	
	return ($allocProcs < $reqProcs);
}



//swf
function getNBAllocProcs($db_in, $index) {
	$alloc_procs = $db_in['data'][$index][4] + 0;
	$req_procs = $db_in['data'][$index][7] + 0;
	
	if($req_procs >= 0 && $alloc_procs >= 0) {
		return $alloc_procs;
	}
	else if ($alloc_procs >= 0) {
		return $alloc_procs;
	}
	else if ($req_procs >= 0) {
		return $req_procs;
	}
	
	return -1;
}

function getRunTime($db_in, $index) {
	//$req_time = $db_in['data'][$index][8] + 0;
	
	//le runtime est l'élément indice 1
	$run_time = $db_in['data'][$index][1] + 0.0;
	
	//if($req_time >= 0 && $run_time >= 0) {
		
		//echo "yo $run_time\n";
		
		return $run_time;
	
	/*
	}
	else if ($run_time >= 0) {
		
		return $run_time;
	}
	else if ($req_time >= 0) {
		//echo "yo $run_time\n";
		//exit(0);
		return $req_time;
	}
	
	return -1;
	*/
}




function getNBReqProcs($db_in, $index) {
	$alloc_procs = $db_in['data'][$index][4] + 0;
	$req_procs = $db_in['data'][$index][7] + 0;
	
	if($req_procs >= 0 && $alloc_procs >= 0) {
		return $req_procs;
	}
	else if ($req_procs >= 0) {
		return $req_procs;
	}
	else if ($alloc_procs >= 0) {
		return $alloc_procs;
	}
	
	reurn -1;
}

function getReqTime($db_in, $index) {
	$req_time = $db_in['data'][$index][4] + 0;
	//$run_time = $db_in['data'][$index][3] + 0;
	
	//if($req_time >= 0 && $run_time >= 0) {
		
		return $req_time;
	
	/*
	}
	else if ($req_time >= 0) {
		
		return $req_time;
	}
	else if ($run_time >= 0) {
		//echo "yo $run_time\n";
		//exit(0);
		return $run_time;
	}
	
	return -1;
	*/
}


function getPi1($db_in, $taskNumber) {
	$req_time = getRunTime($db_in, $taskNumber - 1);
	$req_procs = getNBAllocProcs($db_in, $taskNumber - 1);
	
	if ($req_time >= 0 && $req_procs >= 0) {
		//echo "taskNumber=$taskNumber, req_time = $req_time, req_procs = $req_procs\n";
		//return -1;
	}
	
	return $req_time * $req_procs;
}

function stretch_i($db_out, $db_in, $index_out){
		$taskName = $db_out['data'][$index_out][0];
		$taskNumber = getTaskNumber($taskName);
		
		//$Pi1 = getPi1($db_in, $taskNumber);
		//$Ci =;
		
		$end = $db_out['data'][$index_out][3];
		$arrival = $db_in['data'][$taskNumber - 1][0];
		
		$Ci_ri = $end - $arrival;
		//$ri = $db_out['data'][$index_out][1];
		
		
		$runtime = getRunTime($db_in, $taskNumber - 1);
		
		
		if ($runtime == 0) {
			die ("runtime==0");
		}
		
		//$stretch = ($Ci - $ri) / $Pi1;
		$stretch = ((double)$Ci_ri) / ((double) $runtime);
		
		//echo "end=$end, start=$start, taskNumber=$taskNumber, runtime : ".$runtime.", Ci_ri = $Ci_ri, stretch = $stretch\n";
		
		return $stretch;
}



function get_start_time($db_out, $index) {
	return $db_out['data'][$index][2] + 0.0;
}


function get_end_time($db_out, $index) {
	
	$e = $db_out['data'][$index][3] + 0.0;
	
	echo "endTime = $e\n";
	
	return $e;
}

class LostDataException extends Exception { }

function getMeanImprovement($pFunc, $db_out, $db_out_ref, $nbLignesExpected) {
	$nbLines = count($db_out['data']);
	if ($nbLines < $nbLignesExpected) {
		throw new LostDataException('manque de donnees $db_out');
	}
	
	$nbLinesRef = count($db_out_ref['data']);
	if ($nbLinesRef < $nbLignesExpected) {
		throw new LostDataException('manque de donnees $db_out_ref');
	}
	
	$nbLinesTot = min($nbLines, $nbLinesRef);
	
	$sumImprovement = 0.0;
	
	for($i = 0; $i < $nbLinesTot; $i++) {
		$value = $pFunc($db_out, $i);
		$valueRef = $pFunc($db_out_ref, $i);
		
		$impr = ($value - $valueRef) / $valueRef;
		$sumImprovement += $impr;
	}
	
	
	return $sumImprovement / $nbLinesTot;
}


function numberOfMoldedJobs($db_in, $db_out, $nbLignesExpected) {
	$nbLines = count($db_out['data']);
	
	if ($nbLines < $nbLignesExpected) {
		return -1;
	}
	
	$nbMoldedJobs = 0;
	
	for($i = 0; $i < $nbLignesExpected; $i++) {
		
		if (isMolded($db_in, $db_out, $i)) {
			$nbMoldedJobs++;
		}
		
	}
	
	return $nbMoldedJobs;
	
}

function mean_stretch($db_out, $db_in, $nbLignesExpected) {
	$nbLines = count($db_out['data']);
	
	//echo "il y a $nbLines lignes\n";
	
	if ($nbLines < $nbLignesExpected) {
		return -1;
	}
	
	if($nbLines == 0) {
		exit(-2);
	}
	
	$sumStretch = 0;
	for($i = 0; $i < $nbLignesExpected; $i++) {
		$stretch = stretch_i($db_out, $db_in, $i);
		
		$sumStretch += $stretch;
	}
	
	$mean = ((double) $sumStretch) / (double) ($nbLignesExpected);
	
	return $mean;
}

function mean_stretch_n_tasks($db_out, $db_in, $ntasks) {
	$nbLines = count($db_out['data']);
	
	//echo "il y a $nbLines lignes\n";
	
	if ($nbLines < $ntasks) {
		return -1;
	}
	
	if($nbLines == 0) {
		exit(-2);
	}
	
	$sumStretch = 0;
	for($i = 0; $i < $ntasks; $i++) {
		$stretch = stretch_i($db_out, $db_in, $i);
		
		$sumStretch += $stretch;
	}
	
	$mean = ((double) $sumStretch) / (double) ($ntasks);
	
	return $mean;
}


function max_stretch($db_out, $db_in, $nbLignesExpected) {
	$nbLines = count($db_out['data']);
	
	//echo "il y a $nbLines lignes\n";
	
	if ($nbLines < $nbLignesExpected) {
		return -1;
	}
	
	
	if($nbLines == 0) {
		exit(-2);
	}
	
	$maxStretch = -1.0;
	for($i = 0; $i < $nbLines; $i++) {
		$stretch = stretch_i($db_out, $db_in, $i);
		
		if ($maxStretch < $stretch) {
			$maxStretch = $stretch;
		}
	}
	
	return $maxStretch;
}


function max_stretch_n_tasks($db_out, $db_in, $ntasks) {
	$nbLines = count($db_out['data']);
	
	//echo "il y a $nbLines lignes\n";
	
	if ($nbLines < $ntasks) {
		return -1;
	}
	
	
	if($nbLines == 0) {
		exit(-2);
	}
	
	$maxStretch = -1.0;
	for($i = 0; $i < $ntasks; $i++) {
		$stretch = stretch_i($db_out, $db_in, $i);
		
		if ($maxStretch < $stretch) {
			$maxStretch = $stretch;
		}
	}
	
	return $maxStretch;
}


function cpu_time($db_out, $nbLignesExpected) {
	$nbLines = count($db_out['data']);
	
	if ($nbLines < $nbLignesExpected) {
		return -1;
	}
	
	return $req_time = $db_out['data'][$nbLignesExpected - 1][6] + 0;
}

function mean_cpu_time($db_out, $nbLignesExpected) {
	return cpu_time($db_out, $nbLignesExpected) / $nbLignesExpected;
}

function lowerboundMakespan($db_in, $M) {
	return maximum(sum_surface($db_in) / $M, biggestReqTime($db_in));
}


function echoLine($db, $line) {
	for($i = 0; $i < 8; $i++) {
		echo $db['data'][$line][$i]. " ";
	}
	echo "\n";
}

function getMakespanWithNTasks($db, $NB_TASKS) {
	//global $NB_TASKS;
	
	//$db = dbcsv_open($WLD_OUT);

	if (!isset($db)) {
		//echo "ici\n"; exit(-6);
		return -1.0;
	}

	$nbLines = count($db['data']);
	echo "$nbLines\n";
	
	
	echo "nbLines=$nbLines\n";

	if($nbLines < $NB_TASKS) {
		return -1.0;
	}


	//$sigmaRuntime = 0.0;
	$max_end_time = -1;
	for ($i = 0; $i < $nbLines; $i++) {
		$runtime = $db['data'][$i][4];
		//$nb_procs_used = $db['data'][$i][5];
	
		//$sigmaRuntime += $runtime * $nb_procs_used;
	
		$end_time = $db['data'][$nbLines - 1][3];	
	
		if ($max_end_time < $end_time) {
			$max_end_time = $end_time;
		}
	}
	
	return $max_end_time;
}

function sum_surface($db_in) {
	$nbLines = count($db_in['data']);
	
	$sum = 0.0;
	for($i = 0; $i < $nbLines; $i++) {
		$nbprocs = $db_in['data'][$i][5];
		$requested_time = $db_in['data'][$i][4];
		
		$sum += $nbprocs * $requested_time;
	}
	
	return $sum;
}

function biggestReqTime($db_in) {
	$nbLines = count($db_in['data']);
	
	$max = -1.0;
	for($i = 0; $i < $nbLines; $i++) {
		$requested_time = $db_in['data'][$i][4];
		if ($max < $requested_time) {
			$max = $requested_time;
		}	
	}
	
	return $max;
}

function maximum($x, $y) {
	return ($x > $y ? $x : $y);
}

function load($db_out, $nbProcsInCluster) {
	$nbLines = count($db_out['data']);
	$makespan = getMakespanWithNTasks($db_out, $nbLines);
	echo "makespan = $makespan\n";
	$busySurface = 0.0;
	
	for($i = 0; $i < $nbLines; $i++) {
		$runtime = $db_out['data'][$i][4];
		$nbprocs = $db_out['data'][$i][5];
		
		$surface = $runtime * $nbprocs;
		$busySurface += $surface;
	}
	
	$totalSurface = $nbProcsInCluster * $makespan;
	
	return $busySurface / $totalSurface;
}

?>
