/*
Name: MP Select Mini Web Javascript

Forked from https://github.com/nokemono42/MP-Select-Mini-Web
*/

$(document).ready(function() {
	printerStatus();
	initWebSocket();

	// $(".webcam .img-rounded").click(function() {
	// 	window.sdFilenames.sort();
	// 	console.log(window.sdFilenames);
	// });

	$("#cleargcode").click(function () {
		$("#gCodeLog").html('');
	});

	$(".sd-files .refresh").click(function() {
		if ($("#start_print").hasClass('btn-disable')) {
			return;
		} else {
			refreshSD();
		}
	});

	$(".movement .home").click(function() {
		axis = $(this).attr("data-axis");

		if (axis == 'all') {
			code = 'G28 X0 Y0 Z0';
			comment = 'all axes';
		} else {
			code = 'G28 ' + axis;
			comment = axis + ' axis';
		}

		sendCmd(code, 'Home ' + comment);
		setPositioning = false;
	});

	$(".movement .direction button").click(function() {
		command = 'G1 ';
		movement = $(this).attr("data-movement");
		distance = $(".movement .rate button.active").attr("data-rate");
		axis = $(this).attr("data-axis");
		comment = 'Move ' + axis + ' ' + distance + 'mm';

		if (setPositioning == false) {
			sendCmd('G91', 'Set to Relative Positioning');
			setPositioning = true;
		}

		if (movement == 'up' || movement == 'left') {
			distance = distance * -1;
		}
		if (axis == 'Z' && movement == 'down') {
			comment = 'Raise Z ' + distance + 'mm';
		}
		if (axis == 'Z' && movement == 'up') {
			comment = 'Lower Z ' + distance + 'mm';
		}
		if (axis == 'E' && movement == 'plus') {
			comment = 'Extrude ' + distance + 'mm';
			distance = distance + ' F180';
		}
		if (axis == 'E' && movement == 'minus') {
			sendCmd(command + axis + '-' + distance, 'Retract ' + distance + 'mm');
			return;
		}
		if (movement == 'disable') {
			sendCmd('M18', 'Disable motor lock');
			return;
		}

		sendCmd(command + axis + distance, comment);
	});

	$(".movement .rate button").click(function() {
		$(".movement .rate button").removeClass('active');
		$(this).addClass('active');
	});

	$('#gCodeSend').click(function() {
		gCode2Send = $('#gcode').val();
		if (gCode2Send == '') { return; }
		$('#gcode').val('');
		let type = gCode2Send.startsWith('{') ? 'cmd' : 'code';
		sendCmd(gCode2Send, '',type);
		if (gCode2Send != commandHistory[commandHistory.length-1]) {
			commandHistory.push(gCode2Send);
		}
		clearAndReadyPrompt() 
	});

	function clearAndReadyPrompt() {
		tempCommand=""
		commandPointer=commandHistory.length;
	}

	$(document).on("input", "#gcode", function() {
		if ($('#gcode').val().contains('M23')) return;
		$('#gcode').val($('#gcode').val().toUpperCase());
	})

	$("#gCodeInput").keydown(function(event) {
		if (event.key=="ArrowUp") {
			rollUpThroughHistory();
			return false;
		}
		else if (event.key=="ArrowDown") {
			rollDownThroughHistory();
			return false;
		}
		else if (event.key=="Enter"){
			$('#gCodeSend').click();
			return false;
		}
		else if (event.key=="Escape"){
			 $('#gcode').val(tempCommand);
			clearAndReadyPrompt() 
			return false;
		}
	})		

	var commandHistory=[]
	var commandPointer=0;
	var tempCommand=""
	
	function rollUpThroughHistory() {
		if (commandPointer==0) {
			return;
		}
		if (commandPointer == commandHistory.length && $('#gcode').val() != "") {
			tempCommand = $('#gcode').val();
		}
		let suggestedGcode = commandHistory[commandPointer-1];
		$('#gcode').val(suggestedGcode)		
		commandPointer--;
	}
	
	function rollDownThroughHistory() {
		if (commandPointer == commandHistory.length) {
			return;
		}
		if (commandPointer == commandHistory.length-1) {
			$('#gcode').val(tempCommand);
			tempCommand = ""
		}
		else {
			let suggestedGcode = commandHistory[commandPointer+1];
			$('#gcode').val(suggestedGcode);	
		}
		commandPointer++;
	}

	$("#quickpreheat").click(() => {
		$("#wre").val("215");
		$("#wrp").val("65");
		$("#fanspeed").val(40);
		clearTimeout(timers);
		sendCmd('{C:T0215}{C:P0065}', 'Preheat to 215°C and 65°C', 'cmd');
		sendCmd('M106 S102',' ; Set fan speed to 40%');
		displayTemperatures(215,65)
		clearTimeout(printerStatusTimer)
		printerStatusTimer = setTimeout(printerStatus,3000);
	})

	$("#heatoff").click(() => {
		if ($("#stat").text() == 'Printing') {
			feedback('Cannot disable heat while printing.')
			return;
		}
		$("#wre").val("");
		$("#wrp").val("");
		clearTimeout(timers);
		sendCmd('{C:T0000}{C:P0000}', 'Both Heaters Off', 'cmd');
		displayTemperatures(0,0)
		clearTimeout(printerStatusTimer)
		printerStatusTimer = setTimeout(printerStatus,2000);
	})

	$("#wre").change(function() {
		delaySendTemp($("#wre").val(), 'extruder');
		$("#wre").blur();
	});

	$('#wre').focus(function() {
    var $this = $(this)
        .one('mouseup.mouseupSelect', function() {
            $this.select();
            return false;
        })
        .one('mousedown', function() {
            // compensate for untriggered 'mouseup' caused by focus via tab
            $this.off('mouseup.mouseupSelect');
        })
        .select();
	});

	$("#sete").click(function() {
		delaySendTemp($("#wre").val(), 'extruder');
	});

	$("#clre").click(function() {
		delaySendTemp('0', 'extruder');
	});

	$("#wrp").change(function() {
		delaySendTemp($("#wrp").val(), 'platform');
		$("#wrp").blur();
	});

	$('#wrp').focus(function() {
    var $this = $(this)
        .one('mouseup.mouseupSelect', function() {
            $this.select();
            return false;
        })
        .one('mousedown', function() {
            // compensate for untriggered 'mouseup' caused by focus via tab
            $this.off('mouseup.mouseupSelect');
        })
        .select();
	});

	$("#setp").click(function() {
		delaySendTemp($("#wrp").val(), 'platform');
	});

	$("#clrp").click(function() {
		delaySendTemp('0', 'platform');
	});

	$("#fanspeed").slider({
		min: 30, max: 100, value: 50,
		reversed : true, orientation: 'vertical',
		formatter: function(value) {
			return value + '%';
		}
	});

	$("#fanspeed").on('change', function(slideEvt) {
		delaySendSpeed(slideEvt.value.newValue);
	});

	$("#clrfan").click(function() {
		sendCmd('M106 S0', 'Turn off fan');
	});

	$("form").submit(function() {
		return false;
	});
});

var timers = {};
var setPositioning = false;
var initSDCard = false;
var sdListing = false;
var connected = false;
var connecting = false;
var uploading = false;
var sdFilenames = [];

function pad(num, size) {
	s = '000' + num;
	return s.substr(s.length-size);
}

function scrollConsole() {
	$cont = $("#console");
	$cont[0].scrollTop = $cont[0].scrollHeight;
}

function feedback(output) {
	output = output.replace(/N0 P13 B15/g, '');
	output = output.replace(/N0 P14 B15/g, '');
	output = output.replace(/N0 P15 B13/g, '');
	output = output.replace(/N0 P15 B15/g, '');
	output = output.replace(/N0 P15 B1 /g, '');
	output = output.replace(/echo:/g, '');
	// output = output.replace(/ Size: /g, '<br />Size: ');

	if (output.substring(0, 2) == 'T:' || output.substring(0, 5) == 'ok T:') {
		//Hide temperature reporting
		return;
	}

	console.debug(output);

	// this is a result of printing a file; we safely wait for the file to be opened before calling M24
	if (output.match(/File opened:/g) && output.match(/ Size:/g)) {
		setTimeout(function () {
			sendCmd('M24', `Begin Print`);
		},500)
	}
	
	if (sdListing) {
		if (output.match(/End file list/g)) {
			sdListing = false;
		}
		buildFilenames(output);
		return;
	}
	
	if (output.match(/Begin file list/g)) {
		sdListing = true;
		sdFilenames = [];
		buildFilenames(output);
		return;
	}

	if (output.match(/Change directory /g)) {
		refreshSD()
	}

	output.trim();

	output = output.replace(/\n/g, '<br />');

	// if (output.substring(0, 5) == 'ok N0') {
	// 	output = 'ok';
	// }

	if (output.substring(output.length -6, output.length) == '<br />') {
		output = output.slice(0, -6);
	}

	if (output.substring(0, 10) == 'enqueueing') {
		output = output.substring(11);
		output = output.replace(/"/g, '');

		$("#gCodeLog").append('<p class="text-primary">' + output + ' <span class="text-muted">;</span></p>');
	} else {
		$("#gCodeLog").append('<p class="text-warning">' + output + '</p>');
	}

	scrollConsole();
}

function sendCmd(code, comment, type) {
	if (type === undefined) { type = "code"; }
	
	$("#gCodeLog").append('<p class="text-primary">' + code + ' <span class="text-muted">; ' + comment + '</span></p>');

	$.ajax({ url: 'set?' + type + '=' + code, cache: false }).done();
	//ws.send(code);

	scrollConsole();
}

function startup() {
	if ($("#stat").text() != 'Printing') {
		sendCmd('M563 S5', 'Enable faster Wi-Fi file uploads');
		setTimeout(refreshSD,500);
	}
}

function initWebSocket() {
	url = window.location.hostname;
	if (connecting) return;

	try {
		connecting = true;
		ws = new WebSocket('ws://' + url + ':81');
		feedback('Connecting...');
		ws.onopen = function(a) {
			//console.log(a);
			feedback('Connection established.');
			connected = true;
			connecting = false;
			startup();
		};
		ws.onmessage = function(a) {
			//console.log(a);
			feedback(a.data);
		};
		ws.onclose = function() {
			feedback('Disconnected');
			connected = false;
			// reconnect button?
		}
	} catch (a) {
		//console.log(a);
		feedback('Web Socket Error');
	}
}

function msToTime(duration) {
	var milliseconds = parseInt((duration%1000)/100),
		seconds = parseInt((duration/1000)%60),
		minutes = parseInt((duration/(1000*60))%60),
		hours = parseInt((duration/(1000*60*60))%24);

	hours = (hours < 10) ? "0" + hours : hours;
	minutes = (minutes < 10) ? "0" + minutes : minutes;
	seconds = (seconds < 10) ? "0" + seconds : seconds;

	return hours + ":" + minutes + ":" + seconds + "." + milliseconds;
}

String.prototype.contains = function(it) {
	return this.indexOf(it) != -1;
};

Dropzone.options.mydz = {
	accept: function(file, done) {
		if (file.name.contains('.gc')) {
			//window.startTimer = new Date();

			done();
			$(".print-actions button").addClass('btn-disable');
			$(".movement button").addClass('btn-disable');
			$(".temperature button").addClass('btn-disable');
			uploading = true;
		} else {
			done('Not a valid G-code file.');
		}
	}, init: function() {
		this.on('error', function(file, response) {
			var errorMessage = response.errorMessage;
			$(file.previewElement).find('.dz-error-message').text(errorMessage);
		});

		this.on('addedfile', function() {
			if (this.files[1] != null) {
				this.removeFile(this.files[0]);
			}
		});

		this.on('complete', function(file) {
			uploading = false;
			//File upload duration
			//endTimer = new Date();
			//duration = endTimer - window.startTimer;
			//alert(msToTime(duration));

			$(".print-actions button").removeClass('btn-disable');
			$(".movement button").removeClass('btn-disable');
			$(".temperature button").removeClass('btn-disable');

			//New filename of 21 characters + .gc
			fileParts = file.name.split('.');
			let name = fileParts[0].substring(0, 55);

			if (sdFilenames.indexOf(file.name)) {
				sendCmd('M30 ' + name + '.gc', 'Delete old file');
			}

			setTimeout(function() {
				sendCmd('M566 ' + name + '.gc', '');
			}, 1000);

			setTimeout(function() {
				refreshSD();
			}, 1500);
		});
	}
};

function start_p() {
	// deprecated
}

function cancel_p() {
	$("#start_print").removeClass('btn-disable');
	$("#cancel_print").addClass('btn-disable');
	$("#stat").text('Canceling');
	sendCmd('{P:X}', 'Cancel print', 'cmd');
}

function queuePrinterStatus(delay) {
	if (delay == null) delay = 2000;
	clearTimeout(printerStatusTimer)
	printerStatusTimer = setTimeout(printerStatus,delay);
}

function printerStatus() {
	if (uploading) {
		queuePrinterStatus();
		return;
	}
	$.get("inquiry", function(data, status) {
		//console.log(data);
		// $("#gCodeLog").append('<p class="text-muted">' + data + '</p>');
		// scrollConsole();

		$("#rde").text(data.match( /\d+/g )[0]);
		$("#rdp").text(data.match( /\d+/g )[2]);

		displayTemperatures(data.match( /\d+/g )[1], data.match( /\d+/g )[3]);

		var c = data.charAt(data.length - 1);

		if (c == 'I') {
			$("#stat").text('Idle');
			$("#pgs").css('width', '0%');
			$("#cancel_print").addClass('btn-disable');
			$(".movement button").removeClass('btn-disable');

		} else if (c == 'P') {
			$("#stat").text('Printing');
			$("#pgs").css('width', data.match(/\d+/g )[4] + '%');
			$("#pgs").html(data.match(/\d+/g )[4] + '% Complete');
			$("#cancel_print").removeClass('btn-disable');
			$(".movement button").addClass('btn-disable');
			setPositioning = false;
		} else {
			$("#stat").text('N/A');
		}
		queuePrinterStatus()
	});
}

var currentFile = {};

var printerStatusTimer;

function delaySendTemp(value, device) {
	clearTimeout(timers);
	queuePrinterStatus();
	timers = setTimeout(function() {
		compValue = pad(value, 3);

		if (device == 'extruder') {
			sendCmd('{C:T0' + compValue + '}', 'Set extruder preheat to ' + value + '°C', 'cmd');
			displayTemperatures(value,null);
		}

		if (device == 'platform') {
			sendCmd('{C:P' + compValue + '}', 'Set platform preheat to ' + value + '°C', 'cmd');
			displayTemperatures(null,value);
		}
	}, 250);
}

function delaySendSpeed(value) {
	clearTimeout(timers);
	timers = setTimeout(function() {
		actualSpeed = Math.floor(255 * (value/100));
		sendCmd('M106 S' + actualSpeed, 'Set fan speed to ' + value + '%');
	}, 250);
}

function displayTemperatures(extruder, platform) {
	if (!$('#wre').is(":focus")) $("#wre").val(extruder);
	if (!$('#wrp').is(":focus")) $("#wrp").val(platform);
	if (extruder != "0" && extruder != null) {
			$("#extruder-title").addClass("btn-danger");
			$("#extruder-title").addClass("disabled");
		}
		else if (extruder == "0"){
			$("#extruder-title").removeClass("btn-danger");
			$("#extruder-title").removeClass("disabled");
		}
		if (platform != "0" && platform != null) {
			$("#platform-title").addClass("btn-danger");
			$("#platform-title").addClass("disabled");
		}
		else if (platform == "0"){
			$("#platform-title").removeClass("btn-danger");
			$("#platform-title").removeClass("disabled");
		}
}

function refreshSD() {
	if (initSDCard == false) {
		sendCmd('M21', 'Initialize SD card');
		initSDCard = true;
	}
	sendCmd('M20 L', 'List SD card files');
}

function printFile(filename, lfn) {
	if (!confirm(`Print ${lfn} now?`)) return;
	sendCmd('M23 ' + filename, `Select file ${filename}`);
}

function changeDirectory(filename) {
	filename = filename.replace('./', '');
	if (!filename.contains('/'))
		filename = filename + '/';
	sendCmd('M23 ' + filename, 'Change directory');
	$(".sd-files ul").html('');
}

function deleteFile(item) {
	sendCmd('M30 ' + $(item).parent().text(), 'Delete file');
	
	$(item).parent().fadeOut( "slow", function() {
		$(this).remove();
	});
}

function buildFilenames(output) {
	//TODO refactor this to be concurrent safe
	let filenames = output.split(/\n/g);
	
	filenames.forEach(function(name) {
		if (!(name.includes('Now fresh file:')
			 || name.includes('File opened:')
			 || name == ''
			 || name == '.'
			 || name.includes('Begin file list'))) {
			sdFilenames.push(name);
		}
	});
	
	if (output.match(/End file list/g)) {
		
			function compare( a, b ) {
				var i = a.indexOf(' ');
				var shortlonga = [a.slice(0,i), a.slice(i+1)];
				let sfn = shortlonga[1]
				if (a == '..') {
					sfn = a;
				}
				a = sfn
				var i = b.indexOf(' ');
				var shortlongb = [b.slice(0,i), b.slice(i+1)];
				let sfnb = shortlongb[1]
				if (b == '..') {
					sfnb = b;
				}
				b=sfnb
				if ( a < b ){
					return -1;
				}
				if ( a > b){
					return 1;
				}
				return 0;
			}
		$(".sd-files ul").html('');
		sdFilenames.sort(compare);
		sdFilenames.forEach(function(name) {
			var i = name.indexOf(' ');
			var shortlong = [name.slice(0,i), name.slice(i+1)];
			let sfn = shortlong[0]
			let lfn = shortlong[1]
			if (name == '..') {
				sfn = lfn = name;
			}
			if (name.contains("End file list") || name.contains("ok")) {
				
			}
			else if (name.contains("/") || name.contains("..")) {
				itemHTML = '<li>';
				itemHTML += '<span style="cursor: pointer;" onclick="changeDirectory(\'' + sfn + '\')"><span class="glyphicon glyphicon-folder-open" aria-hidden="true""></span>';
				itemHTML += lfn + '</span>';
				itemHTML += '</li>';
				$('.sd-files ul').append(itemHTML);
			}
			else {
				itemHTML = '<li>';
				itemHTML += `<span class="glyphicon glyphicon-print" aria-hidden="true" style="cursor: pointer;" onclick="printFile('${sfn}','${lfn}')"></span>`;
				itemHTML += `<span class="glyphicon glyphicon-trash" aria-hidden="true" style="cursor: pointer;" onclick="deleteFile(this)"></span><span style="cursor: pointer;" onclick="printFile('${sfn}','${lfn}')">${lfn}</span>`
				itemHTML += '</li>';
				$('.sd-files ul').append(itemHTML);
			}
		});
	}
}
