/*
File: common.js
Author: johnl
Date: 02.06.2021
*/  

function InitCommon() {
    $('[data-toggle="tooltip"]').tooltip({ html:true });

    $(".btn").mouseup(function(){
        $(this).blur();
    }) 
};

function PopUp(infoText)
{
    $("#pop-text").text(infoText);
    $("#popup").fadeIn("fast");
}

$("#popup").click(function() {
    $("#popup").fadeOut("fast");
});

