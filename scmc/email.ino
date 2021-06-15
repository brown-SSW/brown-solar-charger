SMTPSession smtp;
boolean sendEmail(const char* subject, const char* text) {
  ESP_Mail_Session session;
  session.server.host_name = "smtp.gmail.com";
  session.server.port = 465;
  session.login.email = email_address;
  session.login.password = email_password;

  SMTP_Message message;
  message.enable.chunking = true;
  message.sender.name = "Solar Charger";
  message.subject = subject;
  message.sender.email = email_address;

  message.addRecipient("", email_address); //send to self

  message.text.content = text;
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_normal;

  if (!smtp.connect(&session)) {
    Serial.println("error connecting to server with session config");
    return false;
  }
  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("Error sending Email, " + smtp.errorReason());
    return false;
  }
  return true;
}
