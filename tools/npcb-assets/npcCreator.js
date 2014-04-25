var inputCode;
var race;
var gender;
var CutieMarks;
var hairColor0;
var hairColor1;
var bodyColor;
var eyeColor;
var hoofColor;
var Mane;
var Tail;
var Eye;
var Hoof;
var BodySize;
var HornSize;
var options = '<option>say</option><option>giveBits</option><option>give</option><option>goto</option><option>answer</option>';

function addDialogBox(){
	// find the last dialog we created
	var lastDialog = $('npcDialog').last();
	// get the id from that dialog and add 1
	var newDialogId = parseInt(lastDialog.attr('dialogId')) + 1;
	// append a new dialog after that
	lastDialog.after('<npcDialog dialogId="'+newDialogId.toString()+'"><div style="display:none"><select>'+options+'</select><input type="text" placeholder="Something to do"></input></div></npcDialog>');
	// find the div from that dialog so we can apply style
	var newDialog = $('[dialogId="'+newDialogId+'"]').find('div');
	// show it to the user
	newDialog.slideDown();
}

function addMasterLabel(myName){
	myName = myName instanceof Object || myName === undefined ? '' : myName;
	// find the last dialog we created
	var lastMaster = $('npcLabelMaster').last();
	// get the id from that dialog and add 1
	var newLabelId = parseInt(lastMaster.attr('labelId')) + 1;
	// append a new dialog after that
	lastMaster.after('<npcLabelMaster labelId="'+newLabelId+'"><div style="display:none"><div><input value="'+myName+'" class="labelMaster" type="text" placeholder="Label name"></input> <button onclick="addSlaveLabel(\''+newLabelId+'\')">+</button><button onclick="removeSlaveLabel(\''+newLabelId+'\')">-</button></div></div></npcLabelMaster>');
	// find the div from that dialog so we can apply style
	var newLabel = $('[labelId="'+newLabelId+'"]').find('div').first();
	// show it to the user
	newLabel.slideDown();
	addSlaveLabel(newLabelId.toString());
}

function removeMasterLabel(){
	var lastMaster = $('npcLabelMaster').last();
	if(lastMaster.attr('labelId') == '0'){
		lastMaster.find('input').css('background-color', 'red');
		setTimeout(function(){
			lastMaster.find('input').css('background-color', '');
		}, 100);
		return false;
	}
	lastMaster.slideUp('fast', this.remove);
}

function addSlaveLabel(masterLabelId, action, value){
	action = action instanceof Object || action === undefined ? '' : action;
	value = value instanceof Object || value === undefined ? '' : value;
	var masterLabel = $('[labelId="'+masterLabelId+'"]').find('div').first();
	var lastSlave = $(masterLabel).find('npcLabelSlave').last();
	var newLabelId = lastSlave.length != 0 ? parseInt(lastSlave.attr('slaveId')) + 1 : 0;
	masterLabel.append('<npcLabelSlave masterId="'+masterLabelId+'" slaveId="'+newLabelId+'"><div style="display:none"><i class="slaveIndicator">&nbsp;</i><select>'+options+'</select> <input type="text" placeholder="Do something"></input></div></npcLabelSlave>')
	var newSlave = masterLabel.find('[slaveId="'+newLabelId+'"]').find('div').first();
	newSlave.slideDown();
}

function addFilledSlaveLabel(masterLabelId){
	var masterLabel = $('[labelId="'+masterLabelId+'"]').find('div').first();
	var lastSlave = $(masterLabel).find('npcLabelSlave').last();
	var newLabelId = lastSlave.length != 0 ? parseInt(lastSlave.attr('slaveId')) + 1 : 0;
	masterLabel.append('<npcLabelSlave masterId="'+masterLabelId+'" slaveId="'+newLabelId+'"><div style="display:none"><i class="slaveIndicator">&nbsp;</i><select>'+options+'</select> <input type="text" placeholder="Do something"></input></div></npcLabelSlave>')
	var newSlave = masterLabel.find('[slaveId="'+newLabelId+'"]').find('div').first();
	newSlave.slideDown();
}

function removeSlaveLabel(masterLabelId){
	var masterLabel = $('[labelId="'+masterLabelId+'"]').find('div').first();
	var lastSlave = masterLabel.find('npcLabelSlave').last();
	if(lastSlave.attr('slaveId') == '0'){
		lastSlave.find('input').css('background-color', 'red');
		setTimeout(function(){
			lastSlave.find('input').css('background-color', '');
		}, 100);
		return false;
	}
	lastSlave.slideUp('fast', function(){lastSlave.remove()});
}

function addFilledDialogBox(action, str){
	// find the last dialog we created
	var dialogs = $('#dialogs').last();
	var lastDialog = $('npcDialog').last();
	// console.log(lastDialog);
	// get the id from that dialog and add 1
	var newDialogId = lastDialog.length != 0 ? parseInt(lastDialog.attr('dialogId')) + 1 : 0;
	// append a new dialog after that
	dialogs.append('<npcDialog dialogId="'+newDialogId.toString()+'"><div style="display:none"><select>'+options+'</select><input type="text" value="'+str+'" placeholder="Something to do"></input></div></npcDialog>');
	// find the div from that dialog so we can apply style
	var newDialog = $('[dialogId="'+newDialogId+'"]').find('div');
	newDialog.find('select').val(action);
	// show it to the user
	newDialog.slideDown();
}

function getDownloadUrl(str, fileName) {
	if(window.navigator.msSaveOrOpenBlob) {
		var fileData = [str];
		window.navigator.msSaveOrOpenBlob(blobObject, fileName);
	} else {
		var url = "data:application/octet-stream;charset=utf-8," + encodeURIComponent(str);
		return url;
	}
}

function removeDialogBox(){
	var lastDialog = $('npcDialog').last();
	if(lastDialog.attr('dialogId') == '0'){
		lastDialog.find('input').css('background-color', 'red');
		setTimeout(function(){
			lastDialog.find('input').css('background-color', '');
		}, 100);
		return false;
	}
	lastDialog.slideUp('fast', this.remove);
}

function checkInt(e){
		var key = e.which || e.keyCode;
		if (!(key >= 48 && key <= 57) &&
		(key !== 8) &&
		(key !== 9) &&
		(key !== 37) &&
		(key !== 39) &&
		(key !== 46) &&
		(key !== 45)){
			e.preventDefault();
			return false;
		}
}

function onload(){
	// on page load
	$('#addOne').click(addDialogBox);
	$('#removeOne').click(removeDialogBox);
	$('#codeBuilder').click(buildNpc);
	$('#addLabel').click(addMasterLabel);
	$('#removeLabel').click(removeMasterLabel);
	$('#posX').keypress(checkInt);
	$('#posY').keypress(checkInt);
	$('#posZ').keypress(checkInt);
	$('#importButton').click(importNpcCode);
}

function appendLine(txt){
	$('#npcCode').val($('#npcCode').val() + txt + '\n');
}

function validateOptions(){
	return true;
}

function generateExportCode(){
	var npcJson = {
		name: $('#npcName').val(),
		scene: $('#npcScene').val(),
		posX: $('#posX').val(),
		posY: $('#posY').val(),
		posZ: $('#posZ').val(),
		rotX: $('#rotX').val(),
		rotY: $('#rotY').val(),
		rotZ: $('#rotZ').val(),
		rotW: $('#rotW').val(),
		ponyCode: $('#npcPonyCode').val(),
		actions: [],
		labels: []
	};
	$.each($('npcDialog'), function(){
		npcJson.actions.push({action: $(this).find('div').find('select').val(), value: $(this).find('div').find('input').val()});
	});
	$.each($('npcLabelMaster'), function(){
		npcJson.labels.push({labelId: $(this).attr('labelId'), labelName: $(this).find('input').first().val(), slaves: []});
		$.each($(this).find('npcLabelSlave'), function(){
			var slaveAction = $(this).find('select').first().val();
			var slaveValue = $(this).find('input').first().val();
			var masterId = $(this).attr('masterId');
			$.each(npcJson.labels, function(){
				if(this.labelId === masterId){
					this.slaves.push({action: slaveAction, value: slaveValue});
				}
			});
		});
	});
	return JSON.stringify(npcJson).trim();
}

function importNpcCode(){
	try{
		var npcJson = JSON.parse($('#importExportCode').val());
		$('#npcName').val(npcJson.name);
		$('#npcScene').val(npcJson.scene);
		$('#posX').val(npcJson.posX);
		$('#posY').val(npcJson.posY);
		$('#posZ').val(npcJson.posZ);
		$('#rotX').val(npcJson.rotX);
		$('#rotY').val(npcJson.rotY);
		$('#rotZ').val(npcJson.rotZ);
		$('#rotW').val(npcJson.rotW);
		$('#npcPonyCode').val(npcJson.ponyCode);
		$('npcDialog').remove();
		var dialogCounter = 0;
		$.each(npcJson.actions, function(){
			addFilledDialogBox(this.action, this.value);
		});
		$.each(npcJson.labels, function(){

		});
		buildNpc();
	}catch(err){
		$('#importExportCode').val('Error importing code:\n'+err);
	}
}

function buildNpc(){
	try{
		var questId = Math.floor((Math.random()*65535)+200).toString();
		if(validateOptions() != true){
			$('#npcCode').val('');
			appendLine('Invalid options');
		}
		$('#npcCode').val('');
		appendLine('# Metadata\nname ' + $('#npcName').val());
		appendLine('scene ' + $('#npcScene').val());
		appendLine('pos ' + $('#posX').val() + ' ' + $('#posY').val() + ' ' + $('#posZ').val());
		appendLine('rot ' + $('#rotX').val() + ' ' + $('#rotY').val() + ' ' + $('#rotZ').val() + ' ' + $('#rotW').val());
		appendLine('ponyData ' + $('#npcPonyCode').val());
		appendLine('# If you get errors, make sure this quest id is unique.\nquestId ' + questId);
		appendLine('questName ' + $('#npcName').val());
		appendLine('questDescr ' + $('#npcName').val());
		appendLine('\n# Dialog')
		$.each($('npcDialog'), function(){
			if($(this).find('div').find('input').val().trim() != ''){
				appendLine($(this).find('div').find('select').val() + ' ' + $(this).find('div').find('input').val());
			}
		});
		appendLine('end \n\n# Labels');
		$.each($('npcLabelMaster'), function(){
			if($(this).find('input').first().val().trim() != ''){
				appendLine('label ' + $(this).find('input').first().val());
				$.each($(this).find('npcLabelSlave'), function(){
					appendLine($(this).find('div').find('select').val() + ' ' + $(this).find('div').find('input').val());
				});
				appendLine('end');
			}
		});
		$('#importExportCode').val(generateExportCode());
		$('#downloadUrl').attr('download', questId+$('#npcName').val());
		$('#downloadUrl').attr('href', getDownloadUrl($('#npcCode').val(), questId+$('#npcName').val()));
		$('#exportUrl').attr('download', questId+$('#npcName').val()+'-ExportCode');
		$('#exportUrl').attr('href', getDownloadUrl(generateExportCode(), questId+$('#npcName').val()+'-ExportCode'));
	}catch(err){
		$('#npcCode').val('Error building code:\n'+err);
	}
}

function parsePonyData(ponyData){
	inputCode = atob(ponyData); // Browser native - convert from base64 to ascii - not 100% cross-browser, but it is in most. If need be, look for another solution.
	var parsedPonyData = {};

	parsedPonyData.name = inputCode.substr(inputCode.length - 26);
	inputCode = inputCode.substr(name.length);

	parsedPonyData.race = readByte();
	parsedPonyData.gender = readByte();
	parsedPonyData.CutieMarks = [readInt32(), readInt32(), readInt32()]; // to be converted to an actual cutie mark
	parsedPonyData.hairColor0 = {r: readByte(), g: readByte(), b: readByte(), a: 1};
	parsedPonyData.hairColor1 = {r: readByte(), g: readByte(), b: readByte(), a: 1};
	parsedPonyData.bodyColor = {r: readByte(), g: readByte(), b: readByte(), a: 1};
	parsedPonyData.eyeColor = {r: readByte(), g: readByte(), b: readByte(), a: 1};
	parsedPonyData.hoofColor = {r: readByte(), g: readByte(), b: readByte(), a: 1};
	parsedPonyData.Mane = readInt16();
	parsedPonyData.Tail = readInt16();
	parsedPonyData.Eye = readInt16();
	parsedPonyData.Hoof = readInt16();
	parsedPonyData.BodySize = readFloat32();
	parsedPonyData.HornSize = readFloat16(); // Custom, due to readRangedSingle, and in C# returns single, stored in a float.
	return parsedPonyData;
}

function readByte(){
	var toReturn = inputCode[0];
	inputCode = inputCode.substr(1);
	return toReturn;
}

function readInt16(){
	var toReturn = inputCode.charCodeAt(0) | inputCode.charCodeAt(1) << 8;
	inputCode = inputCode.substr(4);
	return toReturn;
}

function readInt32(){
	var toReturn = readInt16() | readInt16() << 16;
	return toReturn;
}

function readFloat16(){
	// We use a float32 array even though it is a float16, because float16 doen't actually exist.
	// If this comes out incorrectly, then try multiplying the output by 2.
	// The original code uses the "single" datatype, which is converted to a float (float32 probably).
	// I am unsure of how the conversions actually work, but this may work fine.
	// To multiply by 2, do "return arr[0] * 2;"
	var arr = new Float32Array(1);
	char = new Uint8Array(arr.buffer);
	for (var i=0; i<2; ++i) char[i] = inputCode.charCodeAt(i);
	inputCode = inputCode.substr(2); 
	return arr[0];
}

function readFloat32(){
	var arr = new Float32Array(1);
	char = new Uint8Array(arr.buffer);
	for (var i=0; i<4; ++i) char[i] = inputCode.charCodeAt(i);
	inputCode = inputCode.substr(4); 
	return arr[0];
}
