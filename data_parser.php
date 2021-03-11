<?php	
	$DayOffset = strtotime($_REQUEST["q"] . " days");
	
	$FileTodayName = "Energy-" . date("d-M-y", $DayOffset) . ".txt";
	
	$Logfile = fopen($FileTodayName, "r") or die("Unable to open file!");
	$DataToTx = fopen("DataToTx.txt", "w") or die("Unable to open file!");
	fwrite($DataToTx, "[");
	
	$MinuteCount = 0;
	$Sum = 0;
	
	while(!feof($Logfile)) {
		$Sum += (int)fgets($Logfile);
		$MinuteCount++;
		
		if($MinuteCount % 5 == 0)
		{
			fwrite($DataToTx, (string)$Sum);			//Add the 5 minute sum
			
			if($MinuteCount != 1440)
			{
				fwrite($DataToTx, ",");
			}
			
			$Sum = 0;
		}		
	}
	fclose($Logfile);
	
	while($MinuteCount < 1440)
	{
		$MinuteCount++;
		
		if($MinuteCount % 5 == 0)
		{
			fwrite($DataToTx, "0");			
			if($MinuteCount != 1440)
			{
				fwrite($DataToTx, ",");
			}
		}
	}
	
	fwrite($DataToTx, "]");	
	fclose($DataToTx);
?>