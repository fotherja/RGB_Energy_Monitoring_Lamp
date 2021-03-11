// some data to be plotted
var x_data = [];
var y_data = [];

for (var i = 0; i < 288; i++) {
   x_data.push(i*5);
   y_data.push(0);
}

var canvas = null;
var StoreScreenYScroll = 0;
	
var DayOffset = 0;
ShiftDateForwards();

const xhrR = new XMLHttpRequest();
xhrR.onreadystatechange = function() {
  if (this.readyState == 4 && this.status == 200) {
    y_data = JSON.parse(this.responseText);
	load_graph();
	Calculate_Data();
  }
};

function load_graph(event)	{
    if(window.myChart)    {
		StoreScreenYScroll = window.pageYOffset;
		window.myChart.destroy();
    }	
	
    var ctx = document.getElementById("canvas").getContext("2d");
    canvas = document.getElementById("canvas");
    window.myChart = new Chart(ctx, {
		type: 'bar',
		data: {
			datasets: [{
				backgroundColor: 'rgba(255, 99, 132, 0.6)',
				borderColor: 'rgba(255, 99, 132, 0.6)',
				borderWidth: 0,
				data: y_data,
			}],
			labels: x_data
		},
		options: {
			legend: {
				display: false
			},
			scales: {
				yAxes: [{
					ticks: {	
						min: 0,
						max: 500
					},
					scaleLabel: {
						display: true,
						labelString: 'Watt hours'
					}
				}],
				xAxes: [{				
					ticks: {
						min: 0,
						max: 1440,
						stepSize: 60
					},							
					scaleLabel: {
						display: true,
						labelString: 'Minutes into day'
					},
					categoryPercentage: 1.0,
					barPercentage: 1.0
				}]
			}
		}
	});
	
	window.scrollTo(0, StoreScreenYScroll);
};

function ShiftDateForwards() {
	if(DayOffset < 0)
	{
		DayOffset += 1;
	}
	document.getElementById("EnergyUsageDate").innerHTML = DateGivenOffset(DayOffset);
	
	var xmlhttp = new XMLHttpRequest();
	xmlhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
			xhrR.open("GET", "DataToTx.txt", true);
			xhrR.send();				
		}
	};
	xmlhttp.open("GET", "data_parser.php?q=" + DayOffset.toString(), true);
	xmlhttp.send();	
};

function ShiftDateBackward() {
	DayOffset -= 1;
	document.getElementById("EnergyUsageDate").innerHTML = DateGivenOffset(DayOffset);	
	
	var xmlhttp = new XMLHttpRequest();
	xmlhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
			xhrR.open("GET", "DataToTx.txt", true);
			xhrR.send();			
		}
	};
	xmlhttp.open("GET", "data_parser.php?q=" + DayOffset.toString(), true);
	xmlhttp.send();	
};

function DateGivenOffset(Offset)	{
	var date = new Date();

	date.setDate(date.getDate() + Offset);	

	return date.toDateString();
}

function Calculate_Data()	{
	var TotalWh = 0;
	
	for (var i = 0; i < 288; i++) {
	   TotalWh += y_data[i];	   
	}

	var TotalkWhFixedDP = (TotalWh/1000).toFixed(2);
	
	document.getElementById("TotalWhUsed").innerHTML = "Total: " + TotalkWhFixedDP.toString() + "kWh";
}





 