//********************************************************************************
// Interface for Sending to Controllers
//********************************************************************************
boolean sendData(struct EventStruct *event)
{
  LoadTaskSettings(event->TaskIndex);
  if (Settings.UseRules)
    createRuleEvents(event->TaskIndex);

  if (Settings.GlobalSync && Settings.TaskDeviceGlobalSync[event->TaskIndex])
    SendUDPTaskData(0, event->TaskIndex, event->TaskIndex);

  if (!Settings.TaskDeviceSendData[event->TaskIndex])
    return false;

  if (Settings.MessageDelay != 0)
  {
    uint16_t dif = millis() - lastSend;
    if (dif < Settings.MessageDelay)
    {
      uint16_t delayms = Settings.MessageDelay - dif;
      char log[30];
      sprintf_P(log, PSTR("HTTP : Delay %u ms"), delayms);
      addLog(LOG_LEVEL_DEBUG_MORE, log);
      unsigned long timer = millis() + delayms;
      while (millis() < timer)
        backgroundtasks();
    }
  }

  LoadTaskSettings(event->TaskIndex); // could have changed during background tasks.

  if (Settings.Protocol)
  {
    byte ProtocolIndex = getProtocolIndex(Settings.Protocol);
    CPlugin_ptr[ProtocolIndex](CPLUGIN_PROTOCOL_SEND, event, dummyString);
  }
  PluginCall(PLUGIN_EVENT_OUT, event, dummyString);
  lastSend = millis();
}

/*********************************************************************************************\
 * Send status info to request source
\*********************************************************************************************/

void SendStatus(byte source, String status)
{
  switch(source)
  {
    case VALUE_SOURCE_HTTP:
      if (printToWeb)
        printWebString += status;
      break;
    case VALUE_SOURCE_SERIAL:
      Serial.println(status);
      break;
  }
}


#if FEATURE_MQTT
/*********************************************************************************************\
 * Handle incoming MQTT messages
\*********************************************************************************************/
// handle MQTT messages
void callback(char* c_topic, byte* b_payload, unsigned int length) {
  char c_payload[384];
  strncpy(c_payload,(char*)b_payload,length);
  c_payload[length] = 0;
  statusLED(true);

  String log;
  log=F("MQTT : Topic: ");
  log+=c_topic;
  addLog(LOG_LEVEL_DEBUG, log);

  log=F("MQTT : Payload: ");
  log+=c_payload;
  addLog(LOG_LEVEL_DEBUG, log);
  
  struct EventStruct TempEvent;
  TempEvent.String1 = c_topic;
  TempEvent.String2 = c_payload;
  byte ProtocolIndex = getProtocolIndex(Settings.Protocol);
  CPlugin_ptr[ProtocolIndex](CPLUGIN_PROTOCOL_RECV, &TempEvent, dummyString);
}


/*********************************************************************************************\
 * Connect to MQTT message broker
\*********************************************************************************************/
void MQTTConnect()
{
  IPAddress MQTTBrokerIP(Settings.Controller_IP);
  MQTTclient.setServer(MQTTBrokerIP, Settings.ControllerPort);
  MQTTclient.setCallback(callback);

  // MQTT needs a unique clientname to subscribe to broker
  String clientid = F("ESPClient");
  clientid += Settings.Unit;
  String subscribeTo = "";

  String LWTTopic = Settings.MQTTsubscribe;
  LWTTopic.replace(F("/#"), F("/status"));
  LWTTopic.replace(F("%sysname%"), Settings.Name);
  
  String log = "";
  boolean MQTTresult = false;

  String msg = F("Connection Lost");
  if ((SecuritySettings.ControllerUser[0] != 0) && (SecuritySettings.ControllerPassword[0] != 0))
    MQTTresult = MQTTclient.connect(clientid.c_str(), SecuritySettings.ControllerUser, SecuritySettings.ControllerPassword, LWTTopic.c_str(), 0, 0, msg.c_str());
  else
    MQTTresult = MQTTclient.connect(clientid.c_str(), LWTTopic.c_str(), 0, 0, msg.c_str());

  if (MQTTresult)
  {
    log = F("MQTT : Connected to broker");
    addLog(LOG_LEVEL_INFO, log);
    subscribeTo = Settings.MQTTsubscribe;
    subscribeTo.replace(F("%sysname%"), Settings.Name);
    MQTTclient.subscribe(subscribeTo.c_str());
    log = F("Subscribed to: ");
    log += subscribeTo;
    addLog(LOG_LEVEL_INFO, log);
  }
  else
  {
    log = F("MQTT : Failed to connected to broker");
    addLog(LOG_LEVEL_ERROR, log);
  }
}


/*********************************************************************************************\
 * Check connection MQTT message broker
\*********************************************************************************************/
void MQTTCheck()
{
  byte ProtocolIndex = getProtocolIndex(Settings.Protocol);
  if (Protocol[ProtocolIndex].usesMQTT)
    if (!MQTTclient.connected())
    {
      if (millis() - lastMQTTReconnectAttempt > 60000) {
        // Reconnect attempts once per minute
        String log = F("MQTT : Connection lost");
        addLog(LOG_LEVEL_ERROR, log);
        connectionFailures++;
        MQTTConnect();
        lastMQTTReconnectAttempt = millis();
      }
    }
    else if (connectionFailures)
      connectionFailures--;
}


/*********************************************************************************************\
 * Send status info back to channel where request came from
\*********************************************************************************************/
void MQTTStatus(String& status)
{
  String pubname = Settings.MQTTsubscribe;
  pubname.replace(F("/#"), F("/status"));
  pubname.replace(F("%sysname%"), Settings.Name);
  MQTTclient.publish(pubname.c_str(), status.c_str(),Settings.MQTTRetainFlag);
}
#endif

