var ready = 0;
Pebble.addEventListener("ready",
    function(e) {
        console.log("Hello world! - Sent from your javascript application.");
        ready = 1;
    }
);

function fixAnswer(answer) {
  return answer.substring(0, 512).replace('′', '\'').replace('″', '"').replace('′', '\'')
}

Pebble.addEventListener('appmessage',
    function(e) {
      console.log('Received message: ' + JSON.stringify(e.payload));
      var query = e.payload.query;
      console.log('Received query: ' + query)

      var req = new XMLHttpRequest();
      req.open('GET', 'https://pebblenow.herokuapp.com/search?q=' + escape(query), true)
      req.onload = function(e) {
        if (req.readyState == 4 && req.status == 200) {
          if(req.status == 200) {
            var response = JSON.parse(req.responseText);
            var answer = fixAnswer(response['answer'])
            console.log('Received answer: ' + answer)
            Pebble.sendAppMessage({ 'answer': answer });
          } else { console.log('Error'); }
        }
      }
      req.send(null);
    }
);
