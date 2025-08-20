// *******************************************************
//  esp32fileServer          by NoRi 2025-08-01
// -------------------------------------------------------
// LF_handler.cpp
// *******************************************************
#include "fileServer.h"
// -------------------------------------------------------
void LF_flServerSetup();
void LF_Dir(AsyncWebServerRequest *request);
void LF_Directory();
void LF_UploadFileSelect();
void LF_handleFileUpload(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final);
void LF_Handle_File_Delete(String encoded_filename);
void LF_Generate_Confirm_Page(String encoded_filename);
void LF_File_Rename();
void LF_Handle_File_Rename(AsyncWebServerRequest *request, String filename, int Args);
bool LF_notFound(AsyncWebServerRequest *request);
void LF_Select_File_For_Function(String title, String function);
void LF_Select_File_For_ViewText();
void LF_View_Text(AsyncWebServerRequest *request, String encoded_filename);
// -------------------------------------------------------
void LFdir_flserverSetup();
void LFdir_handle_chTop();
void LFdir_handle_chUp();
void LFdir_Select_Dir_For_Function(String title, String function);
void LFdir_Generate_Confirm_Page(String encoded_filename);
void LFdir_Handle_chdir(String filename);
void LFdir_Handle_rmdir(String encoded_filename);
void LFdir_Handle_mkdir(AsyncWebServerRequest *request);
void LFdir_DirMake();
void LFdir_DirList();
void LFdir_FilesList();
bool LFdir_notFound(AsyncWebServerRequest *request);
void LFdir_InputNewDirName(String Heading, String Command, String Arg_name);
// -------------------------------------------------------
extern AsyncWebServer server;
extern String webpage;
std::vector<fileinfo> LF_Filenames;
uint32_t LF_startTime, LF_downloadTime = 1, LF_uploadTime = 1;
uint64_t LF_downloadSize, LF_uploadSize;
uint32_t LF_numfiles;
String LfPath = "/";

void LF_flServerSetup()
{
  // Serial.println(__FILE__);

  server.on("/LF_download", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Serial.println("LF_Downloading file...");
    LF_Select_File_For_Function("[DOWNLOAD] for PC", "LF_downloadhandler");
    request->send(200, "text/html", webpage); });

  server.on("/LF_upload", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Serial.println("LF_Uploading file...");
    LF_UploadFileSelect();
    request->send(200, "text/html", webpage); });

  server.on("/LF_handleupload", HTTP_POST, [](AsyncWebServerRequest *request) {}, [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
            { LF_handleFileUpload(request, filename, index, data, len, final); });

  // ********************** LittleFS text file viewer ****************
  server.on("/LF_view_text", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String filename_encoded = "";
              if (request->hasParam("file"))
              { // パラメータ名を 'file' とする場合
                filename_encoded = request->arg("file");
              }
              else
              {
                // エラー処理 or notFound へ
                request->send(400, "text/plain", "Missing file parameter");
                return;
              }
              Serial.println("LF_View_Text requested for: " + filename_encoded);
              LF_View_Text(request, filename_encoded);
              // LF_View_Text 内で request->send するのでここでは不要
            });

  server.on("/LF_vTxt", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Serial.println("LF_vTxt: text file viewer...");
    LF_Select_File_For_ViewText();
    request->send(200, "text/html", webpage); });
  // *********************************************************************

  server.on("/LF_stream", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Serial.println("LF_Streaming file...");
    LF_Select_File_For_Function("[STREAM]", "LF_streamhandler");
    request->send(200, "text/html", webpage); });

  server.on("/LF_rename", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Serial.println("LF_Renaming file...");
    LF_File_Rename();
    request->send(200, "text/html", webpage); });

  server.on("/LF_dir", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Serial.println("LF_File Directory...");
    LF_Dir(request);
    request->send(200, "text/html", webpage); });

  server.on("/LF_delete", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Serial.println("LF_Deleting file...");
    LF_Select_File_For_Function("[DELETE]", "LF_deletehandler");
    request->send(200, "text/html", webpage); });
}

void LF_Dir(AsyncWebServerRequest *request)
{
  LF_Directory();
  webpage = HTML_Header();
  webpage += "<h3>LittleFS: Content (" + LfPath + ")</h3>";
  if (LF_numfiles > 0)
  {
    webpage += "<table class='file-list-table'>";
    webpage += "<tbody>";
    for (int index = 0; index < LF_numfiles; index++)
    {
      webpage += "<tr class='file-entry'>";
      // Name列: Dirの場合はアイコンを追加
      webpage += "<td class='file-name'>";
      if (LF_Filenames[index].ftype == "Dir")
      {
        webpage += "<span style='color: #007bff;'>&#128193;</span> "; // フォルダアイコン 📁
      }
      webpage += LF_Filenames[index].filename;
      webpage += "</td>";
      // Size列: 変更なし
      webpage += "<td class='file-size'>" + LF_Filenames[index].fsize + "</td>";
      webpage += "</tr>";
    }
    webpage += "</tbody>";
    webpage += "</table>";
  }
  else
  {
    webpage += "<p style='text-align: center; margin-top: 20px;'>No files or directories found in " + LfPath + "</p>";
  }
  webpage += HTML_Footer();
}

const String LF_SYSTEM_FILE = "System Volume Information";
void LF_Directory()
{
  LF_numfiles = 0;
  LF_Filenames.clear();
  if (LfPath == "")
    LfPath = "/";
  // Serial.println("LfPath = " + LfPath);
  File root = LittleFS.open(LfPath, "r");

  if (root)
  {
    root.rewindDirectory();
    File file = root.openNextFile();

    while (file)
    {
      String tmp_filename = (String(file.name()).startsWith("/") ? String(file.name()).substring(1) : file.name());

      // パスからファイル名部分だけを抽出 (file.name()がフルパスを返す場合があるため)
      int lastSlash = tmp_filename.lastIndexOf('/');
      if (lastSlash != -1)
      {
        tmp_filename = tmp_filename.substring(lastSlash + 1);
      }

      if (tmp_filename != LF_SYSTEM_FILE && tmp_filename != "") // 空のファイル名も除外
      {
        fileinfo tmp;
        tmp.filename = tmp_filename;
        tmp.ftype = (file.isDirectory() ? "Dir" : "File");
        if (tmp.ftype == "File")
          tmp.fsize = ConvBytesUnits(file.size(), 1);
        else
          tmp.fsize = "";

        LF_Filenames.push_back(tmp);
        LF_numfiles++;
      }
      file = root.openNextFile();
    }
    root.close();
  }
  std::sort(LF_Filenames.begin(), LF_Filenames.end(), compareFileinfo);
}

void LF_UploadFileSelect()
{
  webpage = HTML_Header();
  webpage += "<h3>LittleFS: Select a File to [UPLOAD] to this device (" + LfPath + ")</h3>";
  webpage += "<form method = 'POST' action = '/LF_handleupload' enctype='multipart/form-data'>";
  webpage += "<input type='file' name='filename'><br><br>";
  webpage += "<input type='submit' value='Upload'>";
  webpage += "</form>";
  webpage += HTML_Footer();
}

void LF_handleFileUpload(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
{
  String file = filename;
  if (!index)
  {
    int lastSlash = filename.lastIndexOf('/');
    if (lastSlash != -1)
    {
      file = filename.substring(lastSlash + 1);
    }
    lastSlash = filename.lastIndexOf('\\'); // Windows形式のパス区切りも考慮
    if (lastSlash != -1)
    {
      file = filename.substring(lastSlash + 1);
    }

    if (!file.startsWith("/"))
      file = "/" + file;

    String fullPath = file;
    if (LfPath != "/")
      fullPath = LfPath + file;

    Serial.println("Upload target filename = " + fullPath);
    request->_tempFile = LittleFS.open(fullPath, "w");

    if (!request->_tempFile)
      Serial.println("Error creating file for upload...");

    LF_uploadSize = 0;
    LF_startTime = millis();
  }

  if (request->_tempFile)
  {
    if (len)
    {
      request->_tempFile.write(data, len);
      // Serial.println("Transferred : " + String(len) + " Bytes");
      LF_uploadSize = LF_uploadSize + len;
    }

    if (final)
    {
      request->_tempFile.close();
      LF_uploadTime = millis() - LF_startTime;
      Serial.println("Upload Complete: " + String(request->_tempFile.name()));
      Serial.println("LF_uploadSize = " + String(LF_uploadSize) + " Bytes");
      Serial.println("LF_uploadTime = " + String(LF_uploadTime) + " mSEC");
      request->redirect("/LF_dir");
    }
  }
}

void LF_Handle_File_Delete(String encoded_filename)
{
  webpage = HTML_Header();
  String decoded_filename = urlDecode(encoded_filename);

  String fullPath = decoded_filename;
  if (!fullPath.startsWith("/"))
    fullPath = "/" + fullPath;

  if (LfPath != "/")
    fullPath = LfPath + fullPath;

  Serial.println("LittleFS Delete execute target filename = " + fullPath + " (decoded)");
  File dataFile = LittleFS.open(fullPath, "r");

  if (dataFile)
  {
    dataFile.close(); // ファイルを閉じてから削除
    if (LittleFS.remove(fullPath))
    {
      webpage += "<h3>LittleFS: File '" + decoded_filename + "' in " + LfPath + " has been deleted</h3>";
      webpage += "<a href='/LF_dir'>[OK]</a><br><br>";
    }
    else
    {
      webpage += "<h3>LittleFS: Failed to delete file [ " + decoded_filename + " ] in " + LfPath + "</h3>";
      webpage += "<a href='/LF_delete'>[Back to Select]</a><br><br>";
    }
  }
  else
  {
    webpage += "<h3>LittleFS: File [ " + decoded_filename + " ] in " + LfPath + " does not exist</h3>";
    webpage += "<a href='/LF_delete'>[Back to Select]</a><br><br>";
  }
  webpage += HTML_Footer();
}

void LF_Generate_Confirm_Page(String encoded_filename)
{
  webpage = HTML_Header();
  String decoded_filename = urlDecode(encoded_filename);

  webpage += "<h3>Confirm File Deletion (LittleFS)</h3>";
  webpage += "<p>Are you sure you want to delete the file:</p>";
  webpage += "<p style='font-weight: bold; color: red;'>" + decoded_filename + "</p>";
  // webpage += "<p>in path: " + LfPath + "?</p>";
  webpage += "<p>in path:&nbsp;「&nbsp;" + LfPath + "&nbsp;」&nbsp;?</p>";
  webpage += "<br>";

  // はい（削除実行）ボタン - エンコードされたファイル名を渡す
  webpage += "<a href='/LF_delete_execute~/" + encoded_filename + "' style='padding: 10px 20px; background-color: #dc3545; color: white; text-decoration: none; border-radius: 5px; margin-right: 10px;'>Yes, Delete</a>";

  // いいえ（キャンセル）ボタン
  webpage += "<a href='/LF_delete' style='padding: 10px 20px; background-color: #6c757d; color: white; text-decoration: none; border-radius: 5px;'>No, Cancel</a>";

  webpage += HTML_Footer();
}

void LF_File_Rename()
{
  LF_Directory();
  webpage = HTML_Header();
  webpage += "<h3>LittleFS: Select a Dir/File to [RENAME] (" + LfPath + ")</h3>";
  // methodをGETに変更し、actionを修正 (元のコードに合わせる)
  webpage += "<form action='/LF_renamehandler' method='GET'>";
  webpage += "<table class='file-list-table rename-table'>"; // テーブル開始

  // --- thead (ヘッダー) ---
  webpage += "<thead>";
  webpage += "<tr>";
  // ヘッダーを2列に変更
  webpage += "<th class='rename-select-header'>Select / File Name</th>";
  webpage += "<th class='rename-new-header'>New Filename</th>";
  webpage += "</tr>";
  webpage += "</thead>";

  // --- tbody (ボディ) ---
  webpage += "<tbody>";
  if (LF_numfiles > 0)
  {
    for (int index = 0; index < LF_numfiles; index++)
    {
      String current_filename = LF_Filenames[index].filename;
      String current_ftype = LF_Filenames[index].ftype;
      String radio_id = "choice_" + String(index); // ラジオボタン用の一意なID

      webpage += "<tr class='file-entry'>"; // 各行

      // --- 1列目: Select / File Name ---
      webpage += "<td class='rename-select-cell'>";
      // 非表示のラジオボタン: name='choice', value=ファイル名
      webpage += "<input type='radio' name='choice' value='" + current_filename + "' id='" + radio_id + "' style='display: none;'>";
      // 隠しフィールド: name='oldfile', value=ファイル名 (ハンドラでの取得を容易にするため)
      webpage += "<input type='hidden' name='oldfile' value='" + current_filename + "'>";
      // ラベル (ボタン風): radio_idに対応付け
      webpage += "<label for='" + radio_id + "' class='rename-select-label'>";
      if (current_ftype == "Dir")
      {
        webpage += "<span style='color: #007bff;'>&#128193;</span> "; // フォルダアイコン
      }
      webpage += current_filename; // ファイル名表示
      webpage += "</label>";
      webpage += "</td>";

      // --- 2列目: New Filename ---
      webpage += "<td class='rename-new-name'>";
      // テキスト入力: name='newfile' (各行で同じname属性)
      webpage += "<input type='text' name='newfile' style='width: 95%; box-sizing: border-box;'>";
      webpage += "</td>";

      webpage += "</tr>"; // 行終了
    }
  }
  else
  {
    // ファイルがない場合の表示 (colspanを2に変更)
    webpage += "<tr><td colspan='2' style='text-align: center; padding: 20px;'>No files or directories found in " + LfPath + " to rename.</td></tr>";
  }
  webpage += "</tbody>";
  webpage += "</table><br>";

  // 送信ボタン
  webpage += "<input type='submit' value='Rename Selected'>";
  webpage += "</form>";
  webpage += HTML_Footer();
}

void LF_Handle_File_Rename(AsyncWebServerRequest *request, String filename, int Args)
{
  String oldfilename_form = "";
  String newfilename_form = "";
  webpage = HTML_Header();

  // 1. 選択されたファイル名 (choice の value) を取得
  if (request->hasParam("choice"))
  {
    oldfilename_form = request->arg("choice");
  }
  else
  {
    // choice が選択されていない場合のエラー処理
    webpage += "<h3>LittleFS: Rename Error - No file selected.</h3>";
    webpage += "<a href='/LF_rename'>[Back]</a><br><br>";
    webpage += HTML_Footer();
    request->send(200, "text/html", webpage); // エラーページを送信して終了
    return;
  }

  // 2. 対応する newfile を取得
  //    フォームの引数をループして、選択された oldfile に対応する newfile を探す
  bool found_newfile = false;
  String current_oldfile_check = ""; // ループ内で oldfile を追跡
  for (int i = 0; i < Args; i++)
  {
    String argName = request->argName(i);
    String argValue = request->arg(i);

    if (argName == "oldfile")
    {
      current_oldfile_check = argValue; // 現在の行の oldfile を記録
    }
    else if (argName == "newfile" && current_oldfile_check == oldfilename_form)
    {
      // oldfile が選択されたものと一致し、かつ引数名が newfile なら、それが対応する新しい名前
      newfilename_form = argValue;
      found_newfile = true;
      break; // 見つかったらループを抜ける
    }
    // choice パラメータはここでは無視 (既に取得済みのため)
  }

  // newfile が見つからなかった場合 (通常は発生しないはず)
  if (!found_newfile)
  {
    Serial.println("Error: Could not find corresponding newfile parameter for selected oldfile: " + oldfilename_form);
    webpage += "<h3>LittleFS: Rename Error - Internal error processing form data.</h3>";
    webpage += "<a href='/LF_rename'>[Back]</a><br><br>";
    webpage += HTML_Footer();
    request->send(200, "text/html", webpage); // エラーページを送信して終了
    return;
  }

  Serial.println("Rename requested:");
  Serial.println("  Old filename (from choice): " + oldfilename_form);
  Serial.println("  New filename (from form): " + newfilename_form);
  Serial.println("  Current Path (LfPath): " + LfPath);

  // --- 3. 以降の入力チェックとリネーム処理 ---
  // 入力チェック (newfilename が空でないかもチェック)
  if (oldfilename_form == "" || newfilename_form == "") // newfilenameもチェック
  {
    webpage += "<h3>LittleFS: Rename Error - File selected but new name is missing.</h3>";
    webpage += "<a href='/LF_rename'>[Back]</a><br><br>";
    webpage += HTML_Footer();
    request->send(200, "text/html", webpage); // ★ send を追加
    return;
  }
  if (newfilename_form.indexOf('/') != -1 || newfilename_form.indexOf('\\') != -1)
  {
    webpage += "<h3>LittleFS: Rename Error - New filename cannot contain '/' or '\'.</h3>";
    webpage += "<a href='/LF_rename'>[Back]</a><br><br>";
    webpage += HTML_Footer();
    request->send(200, "text/html", webpage); // ★ send を追加
    return;
  }
  if (oldfilename_form == newfilename_form)
  {
    webpage += "<h3>LittleFS: Rename Error - New filename is the same as the old filename.</h3>";
    webpage += "<a href='/LF_rename'>[Back]</a><br><br>";
    webpage += HTML_Footer();
    request->send(200, "text/html", webpage); // ★ send を追加
    return;
  }

  // フルパスの構築
  String oldfilepath = oldfilename_form;
  if (!oldfilepath.startsWith("/"))
    oldfilepath = "/" + oldfilepath;
  if (LfPath != "/")
    oldfilepath = LfPath + oldfilepath;

  String newfilepath = newfilename_form;
  if (!newfilepath.startsWith("/"))
    newfilepath = "/" + newfilepath;
  if (LfPath != "/")
    newfilepath = LfPath + newfilepath;

  Serial.println("  Old full path: " + oldfilepath);
  Serial.println("  New full path: " + newfilepath);

  // ファイル/ディレクトリの存在確認とリネーム実行
  File currentItem = LittleFS.open(oldfilepath, "r");

  if (currentItem)
  {
    currentItem.close();
    if (LittleFS.exists(newfilepath))
    {
      webpage += "<h3>LittleFS: Rename Error - New filename '" + newfilename_form + "' already exists in " + LfPath + ".</h3>";
      webpage += "<a href='/LF_rename'>[Back]</a><br><br>";
    }
    else
    {
      if (LittleFS.rename(oldfilepath, newfilepath))
      {
        webpage += "<h3>LittleFS: Item '" + oldfilename_form + "' in " + LfPath + " has been renamed to '" + newfilename_form + "'</h3>";
        webpage += "<a href='/LF_dir'>[OK]</a><br><br>";
      }
      else
      {
        webpage += "<h3>LittleFS: Rename Error - Failed to rename '" + oldfilename_form + "' to '" + newfilename_form + "' in " + LfPath + ".</h3>";
        webpage += "<a href='/LF_rename'>[Back]</a><br><br>";
      }
    }
  }
  else
  {
    webpage += "<h3>LittleFS: Rename Error - Original item '" + oldfilename_form + "' not found in " + LfPath + ".</h3>";
    webpage += "<a href='/LF_rename'>[Back]</a><br><br>";
  }

  webpage += HTML_Footer();
}

bool LF_notFound(AsyncWebServerRequest *request)
{
  String filename_encoded = "";
  String url = request->url();

  // ファイル名部分の抽出 ('~/' の後) - ★エンコードされたまま取得
  int separatorIndex = url.indexOf("~/");
  if (separatorIndex != -1)
  {
    filename_encoded = url.substring(separatorIndex + 2);
  }

  // ハンドラ分岐
  if (url.startsWith("/LF_downloadhandler"))
  {
    String dl_filename_decoded = urlDecode(filename_encoded);
    Serial.println("LF_Download handler started for: " + dl_filename_decoded + " (decoded)");
    LF_startTime = millis();

    String fullPath = dl_filename_decoded;
    if (!fullPath.startsWith("/"))
      fullPath = "/" + fullPath;
    if (LfPath != "/")
      fullPath = LfPath + fullPath;

    Serial.println("Download target full path = " + fullPath);
    File file = LittleFS.open(fullPath, "r");

    if (file && !file.isDirectory())
    {
      String contentType = getContentType("download");
      AsyncWebServerResponse *response = request->beginResponse(
          contentType,
          file.size(),
          [file](uint8_t *buffer, size_t maxLen, size_t index) mutable -> size_t
          {
            size_t bytesRead = file.read(buffer, maxLen);
            return bytesRead;
          });
      response->setContentLength(file.size());
      response->addHeader("Content-Disposition", "attachment; filename=\"" + dl_filename_decoded + "\"");
      response->addHeader("Server", "ESP Async Web Server");
      request->send(response);

      LF_downloadSize = file.size();
      LF_downloadTime = millis() - LF_startTime;
      Serial.println("LittleFS download handler initiated...");
    }
    else
    {
      if (file)
        file.close();
      Serial.println("Error: File not found or is a directory: " + fullPath);
      request->send(404, "text/plain", "File not found or is a directory");
    }
    return true;
  }
  else if (url.startsWith("/LF_streamhandler"))
  {
    String stream_filename_decoded = urlDecode(filename_encoded);
    Serial.println("LF_Stream handler started for: " + stream_filename_decoded + " (decoded)");
    LF_startTime = millis();

    String fullPath = stream_filename_decoded;
    if (!fullPath.startsWith("/"))
      fullPath = "/" + fullPath;
    if (LfPath != "/")
      fullPath = LfPath + fullPath;

    Serial.println("Stream target full path = " + fullPath);

    if (LittleFS.exists(fullPath))
    {
      File file = LittleFS.open(fullPath, "r");
      if (file && !file.isDirectory())
      {
        String contentType = getContentType(stream_filename_decoded);
        AsyncWebServerResponse *response = request->beginResponse(LittleFS, fullPath, contentType);
        LF_downloadSize = file.size();
        file.close();
        LF_downloadTime = millis() - LF_startTime;
        request->send(response);
        Serial.println("LittleFS stream handler initiated...");
      }
      else
      {
        if (file)
          file.close();
        Serial.println("Error: Cannot stream directory or file open failed: " + fullPath);
        request->send(404, "text/plain", "Cannot stream directory or file open failed");
      }
    }
    else
    {
      Serial.println("Error: File not found for streaming: " + fullPath);
      request->send(404, "text/plain", "File not found for streaming");
    }
    return true;
  }
  else if (url.startsWith("/LF_delete_confirm"))
  {
    Serial.println("LF_Delete confirm page requested for: " + filename_encoded);
    LF_Generate_Confirm_Page(filename_encoded);
    request->send(200, "text/html", webpage);
    return true;
  }
  else if (url.startsWith("/LF_delete_execute"))
  {
    Serial.println("LF_Delete execute handler started for: " + filename_encoded);
    LF_Handle_File_Delete(filename_encoded);
    request->send(200, "text/html", webpage);
    return true;
  }
  else if (url.startsWith("/LF_renamehandler"))
  {
    Serial.println("LittleFS Rename handler started...");
    LF_Handle_File_Rename(request, "", request->args());
    request->send(200, "text/html", webpage);
    return true;
  }
  return false;
}

void LF_Select_File_For_Function(String title, String function)
{
  LFdir_FilesList();

  webpage = HTML_Header();
  webpage += "<h3>LittleFS: Select a File to " + title + " (" + LfPath + ")</h3>";

  if (LF_numfiles > 0)
  {
    webpage += "<table class='file-list-table'>";
    webpage += "<tbody>";

    for (int index = 0; index < LF_numfiles; index++)
    {
      String Fname_orig = LF_Filenames[index].filename;
      String Fname_encoded = urlEncode(Fname_orig);

      String target_function = function;
      if (function == "LF_deletehandler")
      {
        target_function = "LF_delete_confirm";
      }

      webpage += "<tr class='file-entry'>";
      webpage += "<td class='file-name'>";
      webpage += "<button style='width: 100%; text-align: left; padding: 5px; box-sizing: border-box; white-space: normal; word-break: break-all;'>";

      webpage += "<a href='" + target_function + "~/" + Fname_encoded + "' style='display: block; text-decoration: none; color: inherit;'>" + Fname_orig + "</a>";
      webpage += "</button>";
      webpage += "</td>";
      webpage += "<td class='file-size'>" + LF_Filenames[index].fsize + "</td>";
      webpage += "</tr>";
    }
    webpage += "</tbody>";
    webpage += "</table>";
  }
  else
  {
    webpage += "<p style='text-align: center; margin-top: 20px;'>No files found in " + LfPath + " to " + title + "</p>";
  }
  webpage += HTML_Footer();
}

void LF_Select_File_For_ViewText()
{
  LFdir_FilesList();

  webpage = HTML_Header();
  webpage += "<h3>LittleFS: Select a Text File to View (" + LfPath + ")</h3>";

  // 許可する拡張子のリスト (小文字で定義)
  const std::vector<String> allowedExtensions = {
      ".txt", ".log", ".csv", ".json", ".yaml", ".htm", ".html", ".css", ".js", ".xml", ".md", ".ini", ".conf", ".cfg", ".c", ".h", ".cpp", ".hpp", ".py", ".inc"
      // 必要に応じて他のテキストベースの拡張子を追加する
  };

  int displayedFileCount = 0; // 表示したファイルの数をカウント

  if (LF_numfiles > 0) // LF_numfiles は LFdir_FilesList() で設定されたファイル総数
  {
    webpage += "<table class='file-list-table'>";
    webpage += "<tbody>";

    // 取得した全ファイルをループ
    for (int index = 0; index < LF_numfiles; index++)
    {
      String Fname_orig = LF_Filenames[index].filename;
      String Fname_lower = Fname_orig; // 比較用にファイル名を小文字に変換
      Fname_lower.toLowerCase();

      bool isAllowed = false; // このファイルを表示するかどうかのフラグ

      // 許可リスト内の拡張子と一致するかチェック
      for (const String &ext : allowedExtensions)
      {
        if (Fname_lower.endsWith(ext))
        {
          isAllowed = true; // 一致したらフラグを立ててループを抜ける
          break;
        }
      }

      // 許可された拡張子の場合のみ、テーブルに行を追加
      if (isAllowed)
      {
        displayedFileCount++;
        String Fname_encoded = urlEncode(Fname_orig);
        String link_url = "/LF_view_text?file=" + Fname_encoded;

        webpage += "<tr class='file-entry'>";
        webpage += "<td class='file-name'>";
        webpage += "<button style='width: 100%; text-align: left; padding: 5px; box-sizing: border-box; white-space: normal; word-break: break-all;'>";
        webpage += "<a href='" + link_url + "' style='display: block; text-decoration: none; color: inherit;'>" + Fname_orig + "</a>";
        webpage += "</button>";
        webpage += "</td>";
        webpage += "<td class='file-size'>" + LF_Filenames[index].fsize + "</td>";
        webpage += "</tr>";
      }
    }

    webpage += "</tbody>";
    webpage += "</table>";
  }

  // 表示するテキストファイルが一つもなかった場合のメッセージ
  // LF_numfiles > 0 でも、許可された拡張子のファイルがなければ displayedFileCount は 0 のまま
  if (displayedFileCount == 0)
  {
    // 元々ファイルがなかった場合も、テキストファイルがなかった場合もこのメッセージを表示
    webpage += "<p style='text-align: center; margin-top: 20px;'>No text files found in " + LfPath + "</p>";
  }

  webpage += HTML_Footer();
}

void LF_View_Text(AsyncWebServerRequest *request, String encoded_filename)
{
  String decoded_filename = urlDecode(encoded_filename);
  String fullPath = decoded_filename;
  if (!fullPath.startsWith("/"))
    fullPath = "/" + fullPath;
  if (LfPath != "/")
    fullPath = LfPath + fullPath;

  Serial.println("Viewing text file: " + fullPath);

  File file = LittleFS.open(fullPath, "r");
  if (!file || file.isDirectory())
  {
    if (file)
      file.close();
    webpage = HTML_Header();
    webpage += "<h3>Error: Cannot view file</h3>";
    webpage += "<p>File not found or is a directory: " + decoded_filename + "</p>";
    webpage += "<a href='/LF_dir'>[Back to Directory]</a>";
    webpage += HTML_Footer();
    request->send(404, "text/html", webpage);
    return;
  }

  webpage = HTML_Header();
  webpage += "<h3>Viewing Text: " + decoded_filename + " (" + LfPath + ")</h3>";
  webpage += "<pre>"; // preタグで囲む

  // ファイル内容を読み込んで webpage に追加
  // 大きなファイルの場合、メモリに注意が必要。
  // ここでは一括読み込みをしている。
  while (file.available())
  {
    // 一行ずつ読み込むか、バッファで読み込む
    String line = file.readStringUntil('\n');
    // HTMLエスケープが必要な場合 (例: < > & を表示したい場合)
    line.replace("&", "&amp;");
    line.replace("<", "&lt;");
    line.replace(">", "&gt;");
    webpage += line + "\n"; // 改行も維持
  }
  file.close();

  webpage += "</pre>";
  webpage += "<br><a href='/LF_dir'>[Back to Directory]</a>";
  webpage += "<br>";
  webpage += HTML_Footer();
  request->send(200, "text/html", webpage);
}

// -------------------------------------------------------
// LFdir_* 関数群
// -------------------------------------------------------
void LFdir_flserverSetup()
{
  server.on("/LFdir_chdir", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Serial.println("LFdir_chdir...");
    LFdir_Select_Dir_For_Function("[CHDIR:change directory]", "LFdir_chdirhandler");
    request->send(200, "text/html", webpage); });

  server.on("/LFdir_mkdir", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Serial.println("LFdir_mkdir ...");
    LFdir_DirMake();
    request->send(200, "text/html", webpage); });

  // GETで受ける場合 (LFdir_InputNewDirName からの遷移)
  server.on("/LFdir_mkdirhandler", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        Serial.println("LFdir_mkdir handler (GET) started...");
        LFdir_Handle_mkdir(request);
        request->send(200, "text/html", webpage); });

  server.on("/LFdir_rmdir", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Serial.println("LFdir_rmdir...");
    LFdir_Select_Dir_For_Function("[RMDIR:remove directory]", "LFdir_rmdirhandler");
    request->send(200, "text/html", webpage); });

  server.on("/LFdir_chTop", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Serial.println("change LfPath to Root directory...");
    LFdir_handle_chTop();
    request->send(200, "text/html", webpage); });

  server.on("/LFdir_chUp", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Serial.println("change LfPath to Up directory...");
    LFdir_handle_chUp();
    request->send(200, "text/html", webpage); });
}

void LFdir_handle_chTop()
{
  LfPath = String("/");
  webpage = HTML_Header();
  webpage += "<h3>Changed LittleFS Path to Root Directory (/)</h3>";
  webpage += "<a href='/LF_dir'>[Show Content]</a><br><br>";
  webpage += HTML_Footer();
}

void LFdir_handle_chUp()
{
  String upPath = String("/");

  if (LfPath != "/")
  {
    int i = LfPath.lastIndexOf('/');
    if (i > 0)
    { // ルート("/")の一つ下の階層の場合、iは0になる
      upPath = LfPath.substring(0, i);
    }
    else
    {
      // ルート直下か、予期せぬパス形式の場合はルートに戻す
      upPath = "/";
    }
  }
  // upPath が "/" の場合は変更しない
  if (LfPath != "/")
  {
    LfPath = upPath;
  }

  Serial.println("Changed LfPath to = " + LfPath);
  webpage = HTML_Header();
  webpage += "<h3>Changed LittleFS Path to Up Directory (" + LfPath + ")</h3>";
  webpage += "<a href='/LF_dir'>[Show Content]</a><br><br>";
  webpage += HTML_Footer();
}

void LFdir_Handle_chdir(String encoded_filename)
{
  String filename = urlDecode(encoded_filename);

  webpage = HTML_Header();
  String targetPath = filename;

  if (!targetPath.startsWith("/"))
    targetPath = "/" + targetPath;

  if (LfPath != "/")
    targetPath = LfPath + targetPath;

  Serial.println("Change directory target = " + targetPath);

  // ターゲットパスの正規化 (例: "/dir1//dir2" -> "/dir1/dir2")
  targetPath.replace("//", "/");
  if (targetPath != "/" && targetPath.endsWith("/"))
  {
    targetPath = targetPath.substring(0, targetPath.length() - 1);
  }

  File dir = LittleFS.open(targetPath);
  if (dir && dir.isDirectory())
  { // ディレクトリが存在するか確認
    dir.close();
    LfPath = targetPath;
    // Serial.println("Successfully changed LfPath = " + LfPath);
    webpage += "<h3>Directory changed to '" + LfPath + "'</h3>";
    webpage += "<a href='/LF_dir'>[Show Content]</a><br><br>";
  }
  else
  {
    if (dir)
      dir.close(); // ファイルだった場合や開けなかった場合に閉じる
    Serial.println("Failed to change directory to [ " + targetPath + " ]");
    webpage += "<h3>Error: Directory [ " + filename + " ] not found or is not a directory in " + LfPath + "</h3>";
    webpage += "<a href='/LFdir_chdir'>[Back to Select]</a><br><br>"; // 選択画面に戻るリンク
    webpage += "<a href='/LF_dir'>[Show Current Content]</a><br><br>";
  }
  webpage += HTML_Footer();
}

void LFdir_Handle_rmdir(String encoded_filename)
{
  String filename = urlDecode(encoded_filename);
  webpage = HTML_Header();
  String targetPath = filename;

  if (!targetPath.startsWith("/"))
    targetPath = "/" + targetPath;

  if (LfPath != "/")
    targetPath = LfPath + targetPath;

  Serial.println("Remove directory target = " + targetPath);

  // ルートディレクトリは削除不可
  if (targetPath == "/")
  {
    webpage += "<h3>Error: Cannot remove the root directory.</h3>";
    webpage += "<a href='/LFdir_rmdir'>[Back to Select]</a><br><br>";
    webpage += HTML_Footer();
    return;
  }

  // ディレクトリ削除実行
  if (LittleFS.rmdir(targetPath))
  {
    Serial.println("Removed dir = " + targetPath);
    webpage += "<h3>Directory '" + filename + "' in " + LfPath + " has been removed</h3>";
    webpage += "<a href='/LF_dir'>[OK]</a><br><br>"; // 成功したらDir表示へ
  }
  else
  {
    // 失敗理由を推測 (存在しない、空でない、など)
    String errorMsg = "<h3>Error: Failed to remove directory [ " + filename + " ] in " + LfPath + ".";
    File dir = LittleFS.open(targetPath);
    if (!dir)
    {
      errorMsg += " (Directory not found)";
    }
    else
    {
      dir.close();
      // 空かどうかをチェックするのは難しい場合がある
      errorMsg += " (Directory might not be empty or is write-protected)";
    }
    errorMsg += "</h3>";
    webpage += errorMsg;
    webpage += "<a href='/LFdir_rmdir'>[Back to Select]</a><br><br>";
  }
  webpage += HTML_Footer();
}

void LFdir_Handle_mkdir(AsyncWebServerRequest *request)
{
  webpage = HTML_Header();
  String filename = "";

  // GET または POST パラメータからファイル名を取得
  if (request->hasParam("filename", false))
  { // GET パラメータチェック
    filename = request->arg("filename");
  }
  else if (request->hasParam("filename", true))
  { // POST パラメータチェック
    filename = request->getParam("filename", true)->value();
  }

  Serial.println("Make directory requested name = " + filename);

  // 入力チェック
  if (filename == "")
  {
    webpage += "<h3>Error: Directory name cannot be empty.</h3>";
    webpage += "<a href='/LFdir_mkdir'>[Back]</a><br><br>";
    webpage += HTML_Footer();
    return;
  }
  if (filename.indexOf('/') != -1 || filename.indexOf('\\') != -1)
  {
    webpage += "<h3>Error: Directory name cannot contain '/' or '\'.</h3>";
    webpage += "<a href='/LFdir_mkdir'>[Back]</a><br><br>";
    webpage += HTML_Footer();
    return;
  }

  // フルパス構築
  String fullPath = filename;
  if (!fullPath.startsWith("/"))
    fullPath = "/" + fullPath;

  if (LfPath != "/")
    fullPath = LfPath + fullPath;

  Serial.println("Make directory full path = " + fullPath);

  // ディレクトリ作成実行
  if (LittleFS.mkdir(fullPath))
  {
    Serial.println("Created new dir = " + fullPath);
    webpage += "<h3>Directory '" + filename + "' has been created in " + LfPath + "</h3>";
    webpage += "<a href='/LF_dir'>[OK]</a><br><br>"; // 成功したらDir表示へ
  }
  else
  {
    String errorMsg = "<h3>Error: Failed to create directory [ " + filename + " ] in " + LfPath + ".";
    if (LittleFS.exists(fullPath))
    {
      errorMsg += " (Item with the same name already exists)";
    }
    else
    {
      errorMsg += " (Invalid path or insufficient permissions)";
    }
    errorMsg += "</h3>";
    webpage += errorMsg;
    webpage += "<a href='/LFdir_mkdir'>[Back]</a><br><br>";
  }
  webpage += HTML_Footer();
}

void LFdir_DirMake()
{
  LFdir_InputNewDirName("Make New Directory in " + LfPath, "LFdir_mkdirhandler", "filename");
}

void LFdir_Select_Dir_For_Function(String title, String function)
{
  LFdir_DirList();
  webpage = HTML_Header();
  webpage += "<h3>LittleFS: Select a Directory to " + title + " (" + LfPath + ")</h3>";

  if (LF_numfiles > 0)
  {
    webpage += "<table class='file-list-table'>";
    webpage += "<tbody>";

    for (int index = 0; index < LF_numfiles; index++)
    {
      String Dname_orig = LF_Filenames[index].filename;
      String Dname_encoded = urlEncode(Dname_orig);

      String target_function = function;
      if (function == "LFdir_rmdirhandler")
      {
        target_function = "LFdir_rmdir_confirm";
      }

      webpage += "<tr class='file-entry'>";
      webpage += "<td class='file-name'>";
      webpage += "<button style='width: 100%; text-align: left; padding: 5px; box-sizing: border-box; white-space: normal; word-break: break-all;'>";
      webpage += "<a href='" + target_function + "~/" + Dname_encoded + "' style='display: block; text-decoration: none; color: inherit;'>";
      webpage += "<span style='color: #007bff;'>&#128193;</span> " + Dname_orig;
      webpage += "</a>";
      webpage += "</button>";
      webpage += "</td>";
      webpage += "</tr>";
    }
    webpage += "</tbody>";
    webpage += "</table>";
  }
  else
  {
    webpage += "<p style='text-align: center; margin-top: 20px;'>No sub-directories found in " + LfPath + "</p>";
  }

  webpage += HTML_Footer();
}

void LFdir_Generate_Confirm_Page(String encoded_filename)
{
  webpage = HTML_Header();
  String decoded_filename = urlDecode(encoded_filename);

  webpage += "<h3>Confirm Directory Deletion</h3>";
  webpage += "<p>Are you sure you want to delete the directory:</p>";
  webpage += "<p style='font-weight: bold; color: red;'>" + decoded_filename + "</p>";
  // webpage += "<p>in path: " + LfPath + "?</p>";
  webpage += "<p>in path:&nbsp;「&nbsp;" + LfPath + "&nbsp;」&nbsp;?</p>";
  webpage += "<p style='color: grey;'>Note: Only empty directories can be deleted.</p>";
  webpage += "<br>";

  // はい（削除実行）ボタン
  webpage += "<a href='/LFdir_rmdir_execute~/" + encoded_filename + "' style='padding: 10px 20px; background-color: #dc3545; color: white; text-decoration: none; border-radius: 5px; margin-right: 10px;'>Yes, Delete</a>";

  // いいえ（キャンセル）ボタン
  webpage += "<a href='/LFdir_rmdir' style='padding: 10px 20px; background-color: #6c757d; color: white; text-decoration: none; border-radius: 5px;'>No, Cancel</a>";

  webpage += HTML_Footer();
}

void LFdir_DirList()
{ // 'Dir' type only , not involve 'File'
  LF_numfiles = 0;
  LF_Filenames.clear();

  if (LfPath == "")
    LfPath = "/";
  // Serial.println("Listing directories in: " + LfPath);
  File root = LittleFS.open(LfPath, "r");

  if (root && root.isDirectory()) // ディレクトリとして開けるか確認
  {
    root.rewindDirectory();
    File file = root.openNextFile();
    while (file)
    {
      if (file.isDirectory()) // ディレクトリかどうかをチェック
      {
        String tmp_filename = file.name(); // フルパスが返ることがある
        // パスから最後の部分（ディレクトリ名）を抽出
        int lastSlash = tmp_filename.lastIndexOf('/');
        if (lastSlash != -1)
        {
          tmp_filename = tmp_filename.substring(lastSlash + 1);
        }

        if (tmp_filename != "" && tmp_filename != LF_SYSTEM_FILE) // システムフォルダと空名は除外
        {
          fileinfo tmp;
          tmp.filename = tmp_filename;
          tmp.ftype = "Dir";
          tmp.fsize = ""; // ディレクトリなのでサイズは空

          LF_Filenames.push_back(tmp);
          LF_numfiles++;
        }
      }
      file = root.openNextFile();
    }
    root.close();
  }
  else
  {
    if (root)
      root.close(); // 開けたがディレクトリでなかった場合
    Serial.println("Error opening directory: " + LfPath);
  }
  std::sort(LF_Filenames.begin(), LF_Filenames.end(), compareFileinfo); // 名前順ソート
}

void LFdir_FilesList()
{ // 'File' type only , not involve 'Dir'
  LF_numfiles = 0;
  LF_Filenames.clear();
  if (LfPath == "")
    LfPath = "/";
  // Serial.println("Listing files in: " + LfPath);
  File root = LittleFS.open(LfPath, "r");

  if (root && root.isDirectory())
  {
    root.rewindDirectory();
    File file = root.openNextFile();

    while (file)
    {
      if (!file.isDirectory()) // ファイルかどうかをチェック
      {
        String tmp_filename = file.name();
        int lastSlash = tmp_filename.lastIndexOf('/');
        if (lastSlash != -1)
        {
          tmp_filename = tmp_filename.substring(lastSlash + 1);
        }

        if (tmp_filename != "")
        { // 空のファイル名は除外
          fileinfo tmp;
          tmp.filename = tmp_filename;
          tmp.ftype = "File";
          tmp.fsize = ConvBytesUnits(file.size(), 1);

          LF_Filenames.push_back(tmp);
          LF_numfiles++;
        }
      }
      file = root.openNextFile();
    }
    root.close();
  }
  else
  {
    if (root)
      root.close();
    Serial.println("Error opening directory for file listing: " + LfPath);
  }
  std::sort(LF_Filenames.begin(), LF_Filenames.end(), compareFileinfo);
}

bool LFdir_notFound(AsyncWebServerRequest *request)
{
  String filename_encoded = "";
  String url = request->url();

  // ファイル名部分の抽出 ('~/' の後) - ★エンコードされたまま取得
  int separatorIndex = url.indexOf("~/");
  if (separatorIndex != -1)
  {
    filename_encoded = url.substring(separatorIndex + 2); // エンコードされたファイル名
  }

  if (url.startsWith("/LFdir_chdirhandler"))
  {
    Serial.println("LFdir_chdir handler started for: " + filename_encoded);
    LFdir_Handle_chdir(filename_encoded);
    request->send(200, "text/html", webpage);
    return true;
  }
  else if (url.startsWith("/LFdir_mkdirhandler")) // GETリクエストで処理
  {
    Serial.println("LFdir_mkdir handler (from notFound) started...");
    LFdir_Handle_mkdir(request); // requestオブジェクトを渡して中で引数を解析
    request->send(200, "text/html", webpage);
    return true;
  }
  else if (url.startsWith("/LFdir_rmdir_confirm"))
  {
    Serial.println("LFdir_rmdir confirm page requested for: " + filename_encoded);
    LFdir_Generate_Confirm_Page(filename_encoded);
    request->send(200, "text/html", webpage);
    return true;
  }
  else if (url.startsWith("/LFdir_rmdir_execute"))
  {
    Serial.println("LFdir_rmdir execute handler started for: " + filename_encoded);
    LFdir_Handle_rmdir(filename_encoded);
    request->send(200, "text/html", webpage);
    return true;
  }

  return false;
}

void LFdir_InputNewDirName(String Heading, String Command, String Arg_name)
{
  webpage = HTML_Header();
  webpage += "<h3>" + Heading + "</h3>";
  webpage += "<form method='GET' action='/" + Command + "'>";
  webpage += "New Directory Name: <input type='text' name='" + Arg_name + "' required pattern='[^/\\]+' title='Directory name cannot contain / or \'><br><br>";
  webpage += "<input type='submit' value='Create'>";
  webpage += "</form>";
  webpage += "<br><a href='/LF_dir'>[Cancel and Back to Directory]</a>";
  webpage += HTML_Footer();
}
