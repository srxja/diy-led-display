# Concepts

## HTML Forms:

Think of html forms as letting the user write something, and then sending that something somewhere.
Its minimal shape is:
```
<form action="/submit" method="POST">
  <input type="text" name="msg">
  <button type="submit">Send</button>
</form>
```
- action: where to send the data.
  - /submit: send to the submit path on the same server. so when the user presses this button, it will send a request to ip/submit.
  - This is the path we'll register a handler for, exactly like we did with / (handleroot function)
- method: how to get the data -> GET and POST
- type="text": a single-line text box.
- name="msg" : This is the key under which the typed text gets sent.
    - When the user types "Hello!" and hits submit, the browser sends something like msg=Hello! to the server. On the ESP32 side, you'll ask "what's the value of msg?" and get back "Hello!".
    - The name has to match on both sides, if your HTML says name="msg" but your C++ asks for "message", it is an error.
- button: when click triggers form submission (type = submit)
- GET VS POST
    - Both send data to the server. Difference is:
    - GET —> data is appended to the URL: http://192.168.4.1/submit?msg=Hello. Visible in the address bar. Good for searches, filters, bookmarkable things.
    - POST —> data is sent in the body of the request, hidden from the URL. Good for form submissions with anything with state changes ("create this", "update that").
- What will we use here? POST. Why? We're changing state (updating the displayed message), which is semantically correct for "do something" actions of POST.

## Reading form data on ESP32
When a form is submitted to /submit, the WebServer library parses out the data automatically, which can be accessed via:
```
server.arg("msg") 
```

so our handler function will be like this:
```
void handleSubmit() {
  String newMessage = server.arg("msg");   // fetches the value the user typed
  Serial.print("Received: ");
  Serial.println(newMessage);
  
  message = newMessage;       //message is global ( i hope you've read README)
  scrollPos = 0;              // reset scroll so new message starts from the right
  
  server.send(200, "text/html", "<h1>Got it!</h1><a href='/'>Back</a>"); // sends response back to the phone
}
```
>[!NOTE]
>Next time loop() runs and reads message, it'll see the new text.
>The scroll engine doesn't need to know anything changed, it just reads message every iteration and renders whatever is there.
> This is why globals are useful here: the variable bridges the web handler and the scroll loop.

