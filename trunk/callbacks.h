#ifndef CALLBACKS_H
#define CALLBACKS_H

#define TIMEOUT 3

GstState state;
GstElement *dvdsrc, *d_audio, *d_video, *d_volume;
bool cueFlag;
bool multiCueFlag;
bool multiFileFlag;
bool videoFlag;
bool dvdFlag;
bool preInitFlag;
bool discFlag;
int dvdAudioPads;
int dvdAudioCurrentPad;

WId win;
static Display *display;

CuePlayer *cueplayer = 0;

static gboolean
bus_callback (GstBus     *bus,
		  GstMessage *msg,
		  gpointer    data)
{
	bus = NULL;
	GstElement *playbin = (GstElement *) data;
	GValue *valartist, *valalbum, *valtitle;

  switch (GST_MESSAGE_TYPE (msg)) {

	  case GST_MESSAGE_EOS: {
		g_print ("Конец потока\n");
		if (!multiCueFlag) cueplayer->stopAll();
		break;
	}
	case GST_MESSAGE_STATE_CHANGED: {
		if (gst_element_get_state( GST_ELEMENT(playbin), &state, NULL, GST_SECOND * TIMEOUT) != GST_STATE_CHANGE_SUCCESS)
		{
			g_print ("Аварийный останов.\n");
			cueplayer->stopAll();
		}
		break;
	}
	case GST_MESSAGE_TAG: {
		GstTagList *taglist=gst_tag_list_new();
		gst_message_parse_tag(msg, &taglist);
		valartist = (GValue*)gst_tag_list_get_value_index (taglist, GST_TAG_ARTIST, 0);
		valalbum = (GValue*)gst_tag_list_get_value_index (taglist, GST_TAG_ALBUM, 0);
		valtitle = (GValue*)gst_tag_list_get_value_index(taglist, GST_TAG_TITLE, 0);
		if (!cueFlag)
			cueplayer->setMp3Title(valtitle, valalbum, valartist);
		break;
	}
	case GST_MESSAGE_ELEMENT: {
		if (gst_structure_has_name (msg->structure, "prepare-xwindow-id") && !win)
		{
			gst_x_overlay_set_xwindow_id (GST_X_OVERLAY(GST_MESSAGE_SRC(msg)), win);
			return GST_BUS_DROP;
		}
		break;
	}
	case GST_MESSAGE_ERROR: {
		gchar  *debug;
		GError *error;

		gst_message_parse_error (msg, &error, &debug);
		g_free (debug);

		QMessageBox::critical(0, QObject::trUtf8("Ошибка"), QObject::trUtf8(error->message));
		g_error_free (error);

		gst_element_set_state (playbin, GST_STATE_NULL);
		gst_object_unref (GST_OBJECT (playbin));
		playbin = NULL;
		break;
	}
	default:
	  break;
  }

  return TRUE;
}

static void
cb_typefound (GstElement *typefind,
		  guint       probability,
		  GstCaps    *caps,
		  gpointer    data)
{
	typefind = NULL;
	data = NULL;
	gchar *type;

	type = gst_caps_to_string (caps);
	g_print ("Обнаружен файл типа %s, инфа %d%%\n", type, probability);
	if (!strcmp(type ,"application/x-ape"))
		cueplayer->apeFound(true);
	else
		cueplayer->apeFound(false);
	if (!strcmp(type ,"video/x-matroska") ||
		!strcmp(type ,"video/x-msvideo") ||
		!strcmp(type ,"application/x-3gp") ||
		!strncmp(type ,"video/mpegts", 12) ||
		!strcmp(type ,"video/quicktime"))
		videoFlag = true;
	g_free (type);
}

static void on_pad_added (GstElement *element,
	GstPad *pad,
	gpointer data)
{
	GstCaps *caps;
	const gchar *mime = "NULL";
	//gchar *name;
	//int pres;
	(void) element;
	(void) data;
	guint i;

	GstPad *audiopad, *videopad;

	caps = gst_pad_get_caps (pad);
	g_assert (caps != NULL);
	for (i = 0; i < gst_caps_get_size (caps); ++i) {
		mime = gst_structure_get_name (gst_caps_get_structure (caps, i));
	}

	if (g_strrstr (mime, "audio/x-ac3")) {
		audiopad = gst_element_get_static_pad (d_audio, "sink");
		g_assert(audiopad);
		if (preInitFlag)
		{
			cueplayer->setDvdAudio(gst_pad_get_name(pad), dvdAudioPads);
			dvdAudioPads++;
		}
		//g_print("ПАД ДЕТЕКТ %s\n", gst_pad_get_name(pad)); // Дебаг!
		if (GST_PAD_IS_LINKED (audiopad)) {
			g_object_unref (audiopad);
			return;
		}
		if (!strcmp(gst_pad_get_name(pad), cueplayer->getDvdAudio(dvdAudioCurrentPad)))
			gst_pad_link (pad, audiopad);
		gst_object_unref (audiopad);
	}

	if (g_strrstr (mime, "video/mpeg")) {
		videopad = gst_element_get_static_pad (d_video, "sink");
		g_assert(videopad);
		if (GST_PAD_IS_LINKED (videopad)) {
			g_object_unref (videopad);
			return;
		}
		gst_pad_link (pad, videopad);
		gst_object_unref (videopad);
	}


	gst_caps_unref (caps);
}

GstElement *
make_queue ()
{
  GstElement *queue = gst_element_factory_make ("queue", NULL);
  g_object_set (queue,
	  "max-size-time", (guint64) 3 * GST_SECOND,
	  "max-size-bytes", (guint64) 0,
	  "max-size-buffers", (guint64) 0, NULL);

  return queue;
}

#endif // CALLBACKS_H
