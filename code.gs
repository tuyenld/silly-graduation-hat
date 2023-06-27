var IP = "104.248.243.162"
var POST_URL = "http://" + IP + ":9000";

function myFunction(e) {
  // Get the form to which this script is bound.
  var form = FormApp.getActiveForm();

  // https://medium.com/@eyalgershon/sending-a-webhook-for-each-google-forms-submission-a0e73f72b397
  var allResponses = form.getResponses();
  var latestResponse = allResponses[allResponses.length - 1];
  var response = latestResponse.getItemResponses();
  var payload = {};
  /*
  for (var i = 0; i < response.length; i++) {
      var question = response[i].getItem().getTitle();
      var answer = response[i].getResponse();
      payload[question] = answer;
  }
  */
  // to simplify the json, hardcode the item
  // DON"T CHANGE THE ORDER OF THE QUESTIONS
  var image_path_ascii = response[0].getResponse();
  // ASCII is in range of 0 to 127, so:
  // image_path_ascii = response[0].getResponse().replace(/[^\x00-\x7F]/g, "");
  // image_path_ascii = image_path_ascii.replace(" ", ""); // remove space
  // image_path_ascii = image_path_ascii.toLowerCase() + '.jpg';

  payload["image_path"] = image_path_ascii;
  payload["first_line"] = response[1].getResponse();
  payload["second_line"] = response[2].getResponse();

  var options = {
      "method": "post",
      "muteHttpExceptions": true,
      "contentType": "application/json",
      "payload": JSON.stringify(payload)
  };


  var posRes;
  try {
    posRes = UrlFetchApp.fetch(POST_URL, options);
    //  HTTP response code (e.g. 200 for OK)
    if (posRes.getResponseCode() == 200)
    {
      var posQueue = parseInt(PropertiesService.getScriptProperties().getProperty('posQueue'));
      posQueue = posQueue + 1;
      PropertiesService.getScriptProperties().setProperty('posQueue', posQueue);

      
      form.setConfirmationMessage('The message is on its way. 😊 \n Your position in the queue is ' + posQueue + '.');
    }
    else
      {
        form.setConfirmationMessage('Something wrong has hapened. Please contact the author for assistance.\nError:' + posRes.getContentText())
      }
  } catch (e){
    // hide server IP address
    var errorMes = e.toString();
    errorMes = errorMes.replace(IP, "**.**.**.**");
    //console.log(errorMes);
    form.setConfirmationMessage('Something wrong has hapened. Please contact the author for assistance. \n' + errorMes)
  }
   
}
