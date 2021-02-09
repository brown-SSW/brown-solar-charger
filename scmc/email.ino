SMTPSession smtp;
void setupEmail() {

  smtp.debug(1);

  smtp.callback(smtpCallback);
}
void sendEmail(const char * subject, const char * text) { //without attachment
  _sendEmail(subject, text, "", false);
}
void sendEmail(const char* subject, const char* text, const char* attachment) { //with attachment
  _sendEmail(subject, text, attachment, true);
}
void _sendEmail(const char* subject, const char* text, const char* attachment, boolean attachments) {
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

  if (SD.exists(attachment)) {
    Serial.println("#######files exist");

    SMTP_Attachment att;
    att.descr.filename = attachment;
    att.descr.mime = "text/plain";
    att.file.path = attachment;
    att.file.storage_type = esp_mail_file_storage_type_sd;
    message.addAttachment(att);

  } else {
    Serial.println("ERROR ERROR ERROR ERROR FILES NOT EXISTING ERROR ERROR ERROR");
  }

  if (!smtp.connect(&session)) {
    Serial.println(F("error connecting to server with session config"));
  }
  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("Error sending Email, " + smtp.errorReason());
  }
  Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!sent");
}

void smtpCallback(SMTP_Status status)
{
  /* Print the current status */
  Serial.println("~");
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success())
  {
    Serial.println("------SSSSSSSSSSSSSSSSSSSS----------");
    Serial.printf("Message sent success: %d\n", status.completedCount());
    Serial.printf("Message sent failled: %d\n", status.failedCount());
    Serial.println("------SSSSSSSSSSSSSSSSSSSS----------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      localtime_r(&result.timesstamp, &dt);

      Serial.printf("Message No: %d\n", i + 1);
      Serial.printf("Status: %s\n", result.completed ? "success" : "failed");
      Serial.printf("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      Serial.printf("Recipient: %s\n", result.recipients);
      Serial.printf("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}
