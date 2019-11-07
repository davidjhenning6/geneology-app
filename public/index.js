// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
    // On page-load AJAX Example
    document.getElementById('id01').style.display='block';
    //document.getElementById('id01').style.display='none';


    let chosenFileName = "";
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/checkFiles',   //The server endpoint we are connecting to
        success: function (data) {
                
                let dropOptions = "";
                data.forEach(file => {
                    //console.log(file);
                    if(file.includes(".ged")){
                        dropOptions += '<a href="#">' + file + "</a>";
                    }
                    
                });
                document.getElementById("myDropdown1").innerHTML = dropOptions;
                document.getElementById("myDropdown2").innerHTML = dropOptions;
                document.getElementById("myDropUp").innerHTML = dropOptions;
        },
        
    });

    //add individual drop down
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/checkFiles',   //The server endpoint we are connecting to
        success: function (data) {
                
                let dropOptions = "";
                let i = 1;
                data.forEach(file => {
                    //console.log(file);
                    if(file.includes(".ged")){
                        dropOptions += '<option value="' + file + '">' + file + "</option>";
                        i = i + 1;
                    }
                });
                document.getElementById("filePicker").innerHTML = dropOptions;//select GEDCOM file to display individuals
        },
    });


    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getGEDRow',   //The server endpoint we are connecting to
        success: function (data) {
                
                //console.log(data.table);
                document.getElementById("GEDBody").innerHTML = data.table;
        },
        
    });


    console.log("page loaded");

    // Event listener form replacement example, building a Single-Page-App, no redirects if possible
    $('#someform').submit(function(e){
        e.preventDefault();
        $.ajax({});
    });


    document.getElementById('create').onclick = function () {
                document.getElementById('statusDisplay').innerHTML += "A simple GEDCOM file was created<br>";
                console.log("Dynamic stuff happened");
            };

    document.getElementById('submit').onclick = function () {
                $.ajax({
                    type: 'get',            //Request type
                    dataType: 'json',       //Data type - we will use JSON for almost everything 
                    url: '/getGEDRow',   //The server endpoint we are connecting to
                    success: function (data) {
                            
                            //console.log(data.table);
                            document.getElementById("GEDBody").innerHTML = data.table;
                            document.getElementById('statusDisplay').innerHTML += "File uploaded to server<br>";    
                    },
                    
                });
    };


    $( "select" )
      .change(function() {
        var str = "";
        $( "select option:selected" ).each(function() {
          str += $( this ).text() + " ";
        });
        document.getElementById('statusDisplay').innerHTML += str;
        chosenFileName = "/getGED"+str;
        console.log(chosenFileName);

        $.ajax({
                    type: 'get',            //Request type
                    dataType: 'json',       //Data type - we will use JSON for almost everything 
                    url: chosenFileName,   //The server endpoint we are connecting to
                    success: function (data) {
                            
                            console.log(data);
                            console.log(data.table);

                            document.getElementById("singleFile").innerHTML = data.table;
                            document.getElementById('statusDisplay').innerHTML += "File Selected<br>";    
                    },
                    
                });
      }).trigger( "change" );


      document.getElementById('login').onclick = function () {

                let u = user.value;
                let p = password.value;
                let d = database.value;
                console.log(u);

                let string = "/login"+u+","+p+","+d;


                $.ajax({
                    type: 'get',            //Request type
                    dataType: 'json',       //Data type - we will use JSON for almost everything 
                    url: string,   //The server endpoint we are connecting to
                    success: function (data) {
                            console.log(data);
                            console.log("made it back");
                            if(data.ret == "success"){
                                document.getElementById('id01').style.display='none';
                            }                                                                 
                    },
                });
    };

	
    document.getElementById('gaia').onclick = function () {

                // let u = user.value;
                // let p = password.value;
                // let d = database.value;
                // console.log(u);

                // let string = "/login"+u+","+p+","+d;
		console.log("did i get here?");

                $.ajax({
                    type: 'get',            //Request type
                    dataType: 'json',       //Data type - we will use JSON for almost everything 
                    url: '/createTable',   //The server endpoint we are connecting to
                    success: function (data) {
                            console.log(data);
                                                                                          
                    },
                });
		$.ajax({
                    type: 'get',            //Request type
                    dataType: 'json',       //Data type - we will use JSON for almost everything 
                    url: '/DBdisplay',   //The server endpoint we are connecting to
                    success: function (data) {
                            console.log(data);
                            document.getElementById('statusDisplay').innerHTML += data.ret+"<br>";                  
                    },
                });
    };

    document.getElementById('aether').onclick = function () {

                
                $.ajax({
                    type: 'get',            //Request type
                    dataType: 'json',       //Data type - we will use JSON for almost everything 
                    url: '/deleteTable',   //The server endpoint we are connecting to
                    success: function (data) {
                            console.log(data);
                                                                                          
                    },
                });
    };

    document.getElementById('getData').onclick = function () {

                
                $.ajax({
                    type: 'get',            //Request type
                    dataType: 'json',       //Data type - we will use JSON for almost everything 
                    url: '/DBdisplay',   //The server endpoint we are connecting to
                    success: function (data) {
                            console.log(data);
                            document.getElementById('statusDisplay').innerHTML += data.ret+"<br>";                  
                    },
                });
    };

    document.getElementById('enterQ1').onclick = function () {

                
                $.ajax({
                    type: 'get',            //Request type
                    dataType: 'json',       //Data type - we will use JSON for almost everything 
                    url: '/q1',   //The server endpoint we are connecting to
                    success: function (data) {
                            console.log(data);
                            document.getElementById('statusDisplay2').innerHTML = data.ret;                  
                    },
                });
    };








});