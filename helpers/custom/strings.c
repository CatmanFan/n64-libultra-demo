int language;
char *STR_STAGE;
char *STR_FRAME;
char *STR_CAMPOS;
char *STR_ERROR;
char *STR_REBOOT;

void change_language()
{
	if (language < 0) language = 7;
	if (language > 7) language = 0;

	switch (language)
	{
		default:
		case 0:
			STR_STAGE =  "Language: English\nSelect a stage: %d";
			STR_FRAME =  "Frame %d";
			STR_CAMPOS = "Camera position (%s): %1f";
			STR_ERROR =  "Fatal error at thread %d (priority %d).\nThe software has been terminated.\n\n" \
						 "Error code: %s\n\n" \
						 "Parameters:\n" \
						 "pc       0x%8x\n" \
						 "badvaddr 0x%8x\n";
			STR_REBOOT = "Restarting...";
			break;

		case 1:
			STR_STAGE =  "Langue : français\nChoisir un niveau : %d";
			STR_FRAME =  "Image n° %d";
			STR_CAMPOS = "Position %s de la caméra : %1f";
			STR_ERROR =  "Erreur grave au thread n° %d (priorité fixée à %d).\nLe logiciel a été arrêté.\n\n" \
						 "Code de l'erreur : %s\n\n" \
						 "Paramètres :\n" \
						 "pc       0x%8x\n" \
						 "badvaddr 0x%8x\n";
			STR_REBOOT = "Redémarrage en cours ...";
			break;

		case 6:
			STR_STAGE =  "Tutlayt: Taclḥiyt\nSti aswir: %d";
			STR_FRAME =  "Taɣessa: %d";
			STR_CAMPOS = "Tilawt n tkamirat (%s): %1f";
			// STR_OBJROT = "Amnid (%s):  %1f";
			STR_REBOOT = "Rad isker daɣ uṣnfaṛ.";
			STR_ERROR =  "Tazgelt icqqan ɣ tɣrist %d (tizwiri: %d).\nIsala uṣnfaṛ.\n\n" \
						 "Tabratt n tzgelt: %s\n\n" \
						 "Parameters:\n" \
						 "pc       0x%8x\n" \
						 "badvaddr 0x%8x\n";
			break;

		case 7:
			STR_STAGE =  "言語:日本語\" \
						 "\nステージを選んでください:%d";
			STR_FRAME =  "フレーム %d";
			STR_CAMPOS = "カメラの ばしょ(%s):%1f";
			// STR_CAMROT = "カメラの してん(%s):%1f";
			STR_REBOOT = "再起動しています。\";
			STR_ERROR =  "大変なエラーがスレッド%d(優先度%d)で発生。\" \
						 "\nソフトが終了しました。\" \
						 "\n\nエラーコード:\n%s" \
						 "\n\nくわしく:\n" \
						 "pc       0x%8x\n" \
						 "badvaddr 0x%8x\n";
			break;
	}
};