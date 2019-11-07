'use strict'

// C library API
const ffi = require('ffi');

const mysql = require('mysql');


let connection = null;







// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

app.get('/login:user',function(req, res){

  console.log(req.params.user);
  
  let arr = req.params.user.split(',');
  console.log(arr);
  console.log(arr[1]);

  connection = mysql.createConnection({
      host     : 'dursley.socs.uoguelph.ca',
      user     : arr[0],
      password : arr[1],
      database : arr[2]
  });
  
  let ret = "";


  connection.connect(function(error){
    if(error){
      console.log('connection was not made');
      ret = "error";
    }
    else{
      ret = "success";
    }
    res.send({ret});
  });
  //connection.end();
  console.log("it worked");
  
  

});




// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {

  if(!req.files) {
    console.log("failed to load.");
    return res.status(400).send('No files were uploaded.');

  }
 
  let uploadFile = req.files.uploadFile;
  console.log(uploadFile);
 
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(err);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 

//**************************** C functions ********************


let sharedLib = ffi.Library('./sharedLib', {
  'gedTableRow': ['string', ['string'] ],
  'gedContents': ['string', ['string'] ],
  'sqlFILERow':  ['string', ['string'] ],
  'sqlINDIRow':  ['string', ['string'] ],

});

//*************************************************************






const testFolder = './uploads/';
app.get('/checkFiles', function(req, res){

  
  fs.readdir(testFolder, (err, files) => {
    files.forEach(file => {});
    res.send(files);

  })
});


app.get('/getGEDRow', function(req, res){

  let table = "";

  fs.readdir(testFolder, (err, files) => {
    files.forEach(file => {
      if(file.includes(".ged")){
        table += sharedLib.gedTableRow(file);
      }
    });
    //console.log(table);
    res.send(JSON.stringify({table}));

  })
});


app.get('/getGED:name', function(req, res){

  //let c = JSON.stringify(req.params);
  //console.log(req.params.name);
  let table = "";
  //console.log("hello"+c);

  fs.readdir(testFolder, (err, files) => {
    files.forEach(file => {

      if(file == req.params.name){
        table += sharedLib.gedContents(file);
      }
    });
    //console.log(table);

    res.send(JSON.stringify({table}));

  })
});

app.get('/createTable', function(req, res){

  //connection.connect();



  let createFILE = `create table if not exists FILE(
                          file_id int primary key auto_increment,
                          file_Name varchar(60) not null,
                          source varchar(250) not null,
                          version varchar(10) not null,
                          encoding varchar(10) not null,
                          sub_name varchar(62) not null,
                          sub_addr varchar(256),
                          num_individuals int,
                          num_families int
                      )`;
    //console.log(createFILE);
    connection.query(createFILE, function (err, rows, fields) {
        if (err) {
            console.log("Something went wrong. "+err);
    
        }
    });

    let createINDIVIDUAL = `create table if not exists INDIVIDUAL(
                          ind_id int primary key auto_increment,
                          surname varchar(256) not null,
                          given_name varchar(256) not null,
                          sex varchar(1),
                          fam_size int,
                          source_file int,
			  foreign key (source_file)references FILE(file_id) on delete cascade
                      )`;

	//console.log(createINDIVIDUAL);

    connection.query(createINDIVIDUAL, function (err, rows, fields) {
        if (err) {
            console.log("Something went wrong. "+err);
    
        }
    });

	connection.query("delete from FILE;", function (err, rows, fields) {
        	if (err) {
        	    console.log("Something went wrong. "+err);
    
        	}
	});
	connection.query("delete from INDIVIDUAL;", function (err, rows, fields) {
        	if (err) {
        	    console.log("Something went wrong. "+err);
    
        	}
	});



	//insert into FILE (file_name, source, version, encoding, sub_name, sub_addr, num_individuals, num_families) values ('Hugo','Victor','B+')
	let sqlInsert = "";

	fs.readdir(testFolder, (err, files) => {
	    files.forEach(file => {
			sqlInsert = sharedLib.sqlFILERow(file);
			let arr = sqlInsert.split(',');
			let insertRow = "";


			if(arr[5] == " "){
				 insertRow = "insert ignore into FILE (file_name, source, version, encoding, sub_name, num_individuals, num_families) values ('" + arr[0] + "','" + arr[1] + "','"+ arr[2] + "','"+ arr[3] + "','"+ arr[4] + "','"+ arr[6] + "','"+ arr[7] + "')";
			}else{
				 insertRow = "insert ignore into FILE (file_name, source, version, encoding, sub_name, sub_addr, num_individuals, num_families) values ('" + arr[0] + "','" + arr[1] + "','"+ arr[2] + "','"+ arr[3] + "','"+ arr[4]+ "','" + arr[5] + "','"+ arr[6] + "','"+ arr[7] + "')";
			}
			//console.log(insertRow);
			if(file.includes(".ged")){
				connection.query(insertRow, function (err, rows, fields) {
        				if (err) console.log("Something went wrong. "+err);
    				});
			}

			sqlInsert = sharedLib.sqlINDIRow(file);
			console.log("returned"+sqlInsert);
			if(sqlInsert != "error"){
				let arr2 = sqlInsert.split(';');
				//console.log(arr2);

				arr2.forEach(str => {
					let indiRow = str.split(',');
					if(file.includes(".ged")){
						insertRow = "insert ignore into INDIVIDUAL (surname, given_name, sex, fam_size, source_file) values ('" + indiRow[0] + "','" + indiRow[1] + "','"+ indiRow[2] + "','"+ indiRow[3] + "'," + "(select file_id from FILE where file_Name = '"+ file +"')" + ")";
						console.log("to insert"+insertRow);
						connection.query(insertRow, function (err, rows, fields) {
        						if (err) console.log("Something went wrong. "+err);
    						});
						
					}
			
				

				});
			}


			//let insertRow = "insert into FILE (file_name, source, version, encoding, sub_name, sub_addr, num_individuals, num_families) values ('" + arr[0] + "','" + arr[1] + "','";                           
			
			//connection.query(insertRow, function (err, rows, fields) {
        		//	if (err) {
    			//	        console.log("Something went wrong. "+err);
    
   			//	}
   // });


			
		});
	
	      

	    });

  //connection.end();
});

app.get('/deleteTable', function(req, res){

	connection.query("delete from FILE;", function (err, rows, fields) {
        	if (err) {
        	    console.log("Something went wrong. "+err);
    
        	}
	});
	connection.query("delete from INDIVIDUAL;", function (err, rows, fields) {
        	if (err) {
        	    console.log("Something went wrong. "+err);
    
        	}
	});


});

app.get('/DBdisplay', function(req, res){

	let file_count = 0;
	let indi_count = 0;
	connection.query("select * from FILE;", function (err, result, fields) {
        	if (err) {
        	    console.log("Something went wrong. "+err);
    
        	}else{
			result.forEach( temp => {
				//console.log("file:"+temp);
				file_count = file_count + 1;
			});
			
		}

		console.log("fileCount: "+file_count);

		connection.query("select * from INDIVIDUAL;", function (err, result, fields) {
        	if (err) {
        	    console.log("Something went wrong. "+err);
    
        	}else{
			result.forEach( temp => {
				//console.log("indi:"+temp);
				indi_count = indi_count + 1;
			});
		}
		console.log("IndiCount: "+indi_count);
		let ret = "Database has "+ file_count +" files and "+ indi_count +" individuals";
		console.log(ret);
		res.send({ret});

	});

	});
	

});


app.get('/q1', function(req, res){

	let ret = "";
	connection.query("select * from INDIVIDUAL order by surname;", function (err, result, fields) {
        	if (err) {
        	    console.log("Something went wrong. "+err);
    
        	}else{
			result.forEach( temp => {
				console.log(temp);
				ret += temp.surname+", "+temp.given_name+", "+temp.sex+", "+temp.fam_size+"<br>";
			});
			
		}

		//console.log("fileCount: "+file_count);
		res.send({ret});

	

	});
	

});





//**************************************************************

//Sample endpoint
app.get('/someendpoint', function(req , res){
  res.send({
    foo: "bar"
  });
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);













