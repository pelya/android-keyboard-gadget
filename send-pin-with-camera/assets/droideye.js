//////////////////////////////////////////////
// Global variable define
//////////////////////////////////////////////
var planeWidth = 0;
var planeHeight = 0;
var basicURL = "";
var inStreaming = false;
var picCount = 0;

var audioAPI;
var audioCount = 0;
var audioPlayer;

function CameraSize () {
    this.width = 0;
    this.height = 0;
}
var supportedSize = new Array();
var currentSize = new CameraSize();

var imageDelay = 30;
var timer = null;
var TIMEOUT = 5000;

//////////////////////////////////////////////
// Global function define
//////////////////////////////////////////////
var onImageLoadResult = function(loaded, total, success) {
    if (timer != null) {
        clearTimeout(timer);
        timer = null;
    }
    console.log("result for ", this.src);
    var num = picCount.toString();
    if (this.src.match(num+"$")!=num) {
        console.log("=====>already timeout: " + this.src);
        return;
    }
    if (success) {
        onImageLoadOK();
    } else {
        onImageLoadError();
    }
};
var onImageLoadOK = function() {
    console.log("img load ok");
//    var wid = 0;
//    var hei = 0;
//    if ( planeHeight * currentSize.width / currentSize.height > planeWidth) {
//        wid = planeWidth;
//        hei = math.round(planeWidth * currentSize.height / currentSize.width); 
//    } else {
//        hei = planeHeight;
//        wid = planeHeight * currentSize.width / currentSize.height;  
//    }
//    $("#live_image").width(wid);
//    $("#live_image").height(hei);

    var newDelay = imageDelay / 2;
    if (newDelay >= 30)
        imageDelay = newDelay;
    if ( inStreaming == true)
        setTimeout(refreshLive, imageDelay);  
};

var onImageLoadError = function() {
    console.log("img load error");
    var newDelay = imageDelay * 2;
    if (newDelay > 1000)
        return;
    imageDelay = newDelay;
};

var onQueryDone = function (json) {
    $("#btn_play").button('enable');
    
    var jsonObj = $.evalJSON(json);
    var ret = jsonObj.camsize;
    $("#resolution-choice").empty();
    var resList = ret.split("|");
    currentSize.width = resList[0].split("x")[0];
    currentSize.height = resList[0].split("x")[1];
    var currentSelect = -1;
    for(var i = 1; i < resList.length; i++) {
        var res = resList[i].split("x");
        var newRes = new CameraSize();
        newRes.width = res[0];
        newRes.height = res[1];    
        supportedSize.push(newRes);
        if ( newRes.width == currentSize.width  && newRes.height == currentSize.height) {
            currentSelect = i;
            var newOption = "<option value='" + (i-1) + "'>" + resList[i] + "</option>";
            $("#resolution-choice").append(newOption);
        }
    }
    for(var i = 1; i < resList.length; i++) {
        if ( currentSelect != i) {
            var newOption = "<option value='" + (i-1) + "'>" + resList[i] + "</option>";
            $("#resolution-choice").append(newOption);
        }
    }
    $("#resolution-choice").selectmenu('refresh');
    $("#resolution-choice").bind("change", doChangeRes);  

    $("#debug_msg").html("Connected");

    changeImageWH(currentSize.width, currentSize.height);

    var quality = jsonObj.quality;
    $("#slider").val(quality).slider("refresh");
};

var onHttpError = function () {
    $("#debug_msg").html("Can't connected with phone, please refresh web page!");   
    $("#btn_play").button('disable'); 
};

function getFunc(f, arg) {
    return function() {
        return f(arg);
    }
}
var refreshLive = function() {
    picCount = picCount + 1;
    $("#live_image").attr("src", basicURL + "stream/live.jpg?id=" + picCount);
    $("#video_plane").waitForImages(null, onImageLoadResult );
    timer = setTimeout(getFunc(function(picId){
        console.log("====>timeout: " + picId);
        if ( inStreaming == true)
            refreshLive();  
    }, picCount), TIMEOUT);
    console.log("picCount:" + picCount + ", delay:" + imageDelay);
};

var playClick = function () {
    if  ( inStreaming == false) {
        inStreaming = true;
        $("#btn_play").val("Stop").button("refresh");
        $("#resolution-choice").selectmenu("disable");
        $("#checkbox-audio").checkboxradio('disable');
        
        refreshLive();

        if ( $("#checkbox-audio").is(":checked") ) {
            var newClip = {'url':'stream/live.mp3?id='+audioCount,'autoplay':true};
            audioCount ++;
            audioPlayer.play(newClip);
        }
    } else {
        inStreaming = false;
        $("#btn_play").val("Play").button("refresh");
        $("#resolution-choice").selectmenu("enable");
        $("#checkbox-audio").checkboxradio('enable');
        audioPlayer.stop();
        audioPlayer.close();
    }
};

var onSetupOK = function() {
//    var targetIndex = $("#resolution-choice").val();
//    currentSize = supportedSize[targetIndex]; 
};

function changeImageWH(width, height) {
    planeHeight = height;
    planeWidth = width;
//    $("#video_plane").width(planeWidth);
//    $("#video_plane").height(planeHeight);
    $("#live_image").width(planeWidth);
    $("#live_image").height(planeHeight);
//    $("#player").width(planeWidth);
    //$("#player").height(hei);
}
var doChangeRes = function () {
    var targetIndex = $("#resolution-choice").val();
    var wid = supportedSize[targetIndex].width;
    var hei = supportedSize[targetIndex].height; 
    currentSize = supportedSize[targetIndex]; 
    changeImageWH(wid, hei);
    $.ajax({
        type: "GET",
        url: basicURL + "cgi/setup",
        cache: false,
        data: "wid=" + wid + "&hei=" + hei,
        success: onSetupOK
    });
};

var initAudioPlayer = function () {
    // install flowplayer into container
    // http://flash.flowplayer.org/

    $f("player", "flowplayer-3.2.15.swf", {
        plugins: {
            controls: {
                fullscreen: false,
                height: 30,
                autoHide: false,
                play: false,
            }
        },
        clip: {
            autoPlay: false,
            url: "stream/live.mp3",
        }
    });

    audioPlayer = $f();
};

$(document).on("pageinit", "#page_main", function() {
    basicURL = $(location).attr('href');
        
    var screenHeight = $(window).height();
    var screenWidth = $(window).width();
    planeHeight = Math.round( screenHeight * 0.5);
    planeWidth = Math.round( screenWidth * 0.80);

//    $("#video_plane").height(planeHeight);
//    $("#video_plane").width(planeWidth);

    $("#btn_play").button('disable');
    $("#btn_play").bind("click", playClick);
    $("#rotate0").bind("click", function(){rotate(0);});
    $("#rotate90").bind("click", function(){rotate(90);});
    $("#rotate180").bind("click", function(){rotate(180);});
    $("#rotate270").bind("click", function(){rotate(270);});
    $("#btn_autofocus").bind("click", autoFocus);
    $("#slider").on("slidestop", changeQuality);
    $("#btn_dimscreen").bind("click", dimScreen);
    
    initAudioPlayer();

    $.ajax({
        type: "GET",
        url: basicURL + "cgi/query",
        cache: false,
        error: onHttpError,
        success: onQueryDone
    });
});

function rotate(degree) {
    var r = "rotate(" + degree + "deg)";
    var css = {"-webkit-transform":r,
        "-moz-transform":r,
        "-o-transform":r,
        "-ms-transform":r,
        "transform":r};
//    $("#live_image").css(css);

//        changeImageWH(planeHeight, planeWidth);
//        $("#live_image").width(planeHeight);
//        $("#live_image").height(planeWidth);

    $("#live_image").rotate(degree);
//    if (degree == 90 || degree == 270)
//        $("#video_plane").height(planeWidth);
//    else
//        $("#video_plane").height(planeHeight);

    $.ajax({
        type: "GET",
        url: basicURL + "cgi/rotate",
        cache: false,
        data: "degree=" + degree,
        success: function(){
            console.log("successfully rotate " + degree);
        }
    });
}

function autoFocus() {
    $.ajax({
        type: "GET",
        url: basicURL + "cgi/autofocus",
        cache: false,
        success: function(){
            console.log("successfully auto focus");
        }
    });
}

function changeQuality(e) {
    var quality = $(e.target).val();
    if (!quality) return;
    console.log("change quality:", quality);
    $.ajax({
        type: "GET",
        url: basicURL + "cgi/changequality",
        cache: false,
        data: "quality=" + quality,
        success: function(){
            console.log("successfully changed quality to " + quality);
        }
    });
}

function dimScreen() {
    $.ajax({
        type: "GET",
        url: basicURL + "cgi/dimscreen",
        cache: false,
        success: function(){
            console.log("successfully dim screen");
        }
    });
}


//////////////////////////////////////////////
// Top level code define
//////////////////////////////////////////////
$(document).ready(function(){

});

