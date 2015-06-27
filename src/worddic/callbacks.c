#include <glib.h>

#include "worddic.h"
#include "worddic_dicfile.h"
#include "../common/dicfile.h"

G_MODULE_EXPORT gboolean on_search_results_button_release_event(GtkWidget *text_view,
                                                                GdkEventButton *event,
                                                                worddic *worddic) {
  GtkTextIter mouse_iter;
  gint x, y;
  gint trailing;
  gunichar kanji;
  /*
  if (event->button != 1) return FALSE;
  
  gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW (text_view), 
					GTK_TEXT_WINDOW_WIDGET,
					event->x, event->y, &x, &y);

  gtk_text_view_get_iter_at_position(GTK_TEXT_VIEW(text_view), &mouse_iter, &trailing, x, y);
  kanji = gtk_text_iter_get_char(&mouse_iter);
  if ((kanji != 0xFFFC) && (kanji != 0) && (isKanjiChar(kanji) == TRUE)) {
  }
  */
  return FALSE;
}

/*
 * Update the cursor image if the pointer is above a kanji. 
 */
G_MODULE_EXPORT gboolean on_search_results_motion_notify_event(GtkWidget *text_view,
                                                               GdkEventMotion *event){
  gint x, y;
  GtkTextIter mouse_iter;
  gunichar kanji;
  gint trailing;
  
  gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(text_view), 
                                        GTK_TEXT_WINDOW_WIDGET,
                                        event->x, event->y, &x, &y);
  
  gtk_text_view_get_iter_at_position(GTK_TEXT_VIEW(text_view),
                                     &mouse_iter, &trailing,
                                     x , y);
  
  kanji = gtk_text_iter_get_char(&mouse_iter);

  // Change the cursor if necessary
  if ((isKanjiChar(kanji) == TRUE)) {
    gdk_window_set_cursor(gtk_text_view_get_window(GTK_TEXT_VIEW(text_view),
                                                   GTK_TEXT_WINDOW_TEXT),
                          cursor_selection);
  }
  else{
    gdk_window_set_cursor(gtk_text_view_get_window(GTK_TEXT_VIEW(text_view),
                                                   GTK_TEXT_WINDOW_TEXT),
                          cursor_default);
  }
  
  return FALSE;
}

/**
   search entry activate signal callback:
   Search in the dictionaries the entered text in the search entry
   and put the results in the search result textview buffer
*/
G_MODULE_EXPORT void on_search_activate(GtkEntry *entry, worddic *worddic){
  cur_matches = 0;

  gint match_criteria_jp  = worddic->match_criteria_jp;
  gint match_criteria_lat  = worddic->match_criteria_lat;
  
  //get the expression to search from the search entry
  gchar *entry_text = gtk_entry_get_text(entry);

  //detect is the search is in japanese
  gboolean is_jp = detect_japanese(entry_text);

  //modify the expression with anchors if there are search criteria
  GString *entry_string = g_string_new(entry_text);
  if(is_jp){
    if(match_criteria_jp == EXACT_MATCH){
      entry_string = g_string_prepend_c(entry_string, '^');
      entry_string = g_string_append_c(entry_string, '$');
    }
    else if(match_criteria_jp == START_WITH_MATCH){
      entry_string = g_string_prepend_c(entry_string, '^');
    }
  
    else if(match_criteria_jp == END_WITH_MATCH){
      entry_string = g_string_append_c(entry_string, '$');
    }
  }
  else{
    if(match_criteria_lat == EXACT_MATCH){
      entry_string = g_string_prepend_c(entry_string, '^');
      entry_string = g_string_append_c(entry_string, '$');      
    }
    else if(match_criteria_lat == WORD_MATCH){
        entry_string = g_string_prepend(entry_string, "\\b");
        entry_string = g_string_append(entry_string, "\\b");
    }
  }

  gchar *entry_text_raw = strdup(entry_text);
  entry_text = entry_string->str;
  
  //get the search result text entry to display matches
  GtkTextBuffer *textbuffer_search_results = 
    (GtkTextBuffer*)gtk_builder_get_object(worddic->definitions, 
                                           "textbuffer_search_results");
  
  gboolean deinflection   = worddic->conf->verb_deinflection;

  //search in the dictionaries
  GSList *dicfile_node;
  WorddicDicfile *dicfile;
  dicfile_node = worddic->conf->dicfile_list;
  dicfile = dicfile_node->data;
  GList *results=NULL;               //matched dictionary entries
  GList *l = NULL;                   //browse results

  //clear the display result buffer
  gtk_text_buffer_set_text(textbuffer_search_results, "", 0);
  
  //in each dictionaries
  while (dicfile_node != NULL) {
    GList *results = NULL;
    dicfile = dicfile_node->data; 
    
    if(is_jp){
      //search for deinflections
      if(deinflection){
        results = g_list_concat(results, search_verb_inflections(dicfile, entry_text_raw));
      }

      //search hiragana on katakana
      if (worddic->conf->search_hira_on_kata &&
          hasKatakanaString(entry_text)) {
        gchar *hiragana = kata2hira(entry_text);
        results = g_list_concat(results, dicfile_search(dicfile, hiragana));
        g_free(hiragana);  //free memory
      }
    
      //search katakana on hiragana
      if (worddic->conf->search_kata_on_hira &&
          hasHiraganaString(entry_text)) { 
        gchar *katakana = hira2kata(entry_text);
        results = g_list_concat(results, dicfile_search(dicfile, katakana));
        g_free(katakana); //free memory
      }
    }

    //standard search
    results = g_list_concat(results, dicfile_search(dicfile, entry_text));

    //print
    print_entry(textbuffer_search_results, results, worddic);

    //free the dicresult's match
    g_list_free_full(results, dicresult_free_match);
    
    //get the next node in the dic list
    dicfile_node = g_slist_next(dicfile_node);
  }

  //free memory
  g_free(entry_text_raw);
  g_string_free(entry_string, TRUE);
}

//////////////////////
//Menuitems callbacks

//Edit
///Preferences
G_MODULE_EXPORT void on_menuitem_prefs_activate(GtkMenuItem *menuitem, worddic *worddic){
  GtkDialog *prefs = GTK_DIALOG(gtk_builder_get_object(worddic->definitions, 
                                                       "prefs"));
  //set size and display the preference window
  gtk_window_set_default_size(GTK_WINDOW(prefs), 320, 220);
  gtk_widget_show_all ((GtkWidget*)prefs);
}

//Search
///Japanese
G_MODULE_EXPORT void on_menuitem_search_japanese_exact_activate (GtkMenuItem *menuitem, 
                                                                 worddic *worddic){
  worddic->match_criteria_jp = EXACT_MATCH;
}

G_MODULE_EXPORT void on_menuitem_search_japanese_start_activate (GtkMenuItem *menuitem, 
                                                                 worddic *worddic){
  worddic->match_criteria_jp = START_WITH_MATCH;
}

G_MODULE_EXPORT void on_menuitem_search_japanese_end_activate (GtkMenuItem *menuitem, 
                                                               worddic *worddic){
  worddic->match_criteria_jp = END_WITH_MATCH;
}

G_MODULE_EXPORT void on_menuitem_search_japanese_any_activate (GtkMenuItem *menuitem, 
                                                               worddic *worddic){
  worddic->match_criteria_jp = ANY_MATCH;
}

///Latin
G_MODULE_EXPORT void on_menuitem_search_latin_whole_expressions_activate (GtkMenuItem *menuitem, 
                                                                          worddic *worddic){
  worddic->match_criteria_lat = EXACT_MATCH;
}

G_MODULE_EXPORT void on_menuitem_search_latin_whole_words_activate (GtkMenuItem *menuitem, 
                                                                    worddic *worddic){
  worddic->match_criteria_lat = WORD_MATCH;
}

G_MODULE_EXPORT void on_menuitem_search_latin_any_matches_activate (GtkMenuItem *menuitem, 
                                                                    worddic *worddic){
  worddic->match_criteria_lat = ANY_MATCH;
}


//Help
///About
G_MODULE_EXPORT void on_menuitem_help_about_activate (GtkMenuItem *menuitem, 
                                                      worddic *worddic){
  GtkWindow *window_about = (GtkWindow*)gtk_builder_get_object(worddic->definitions,
                                                               "aboutdialog");
  gtk_dialog_run(GTK_DIALOG(window_about));
  gtk_widget_hide (GTK_WIDGET(window_about)); 
}
