//gcc parse_line.c $(pkg-config --cflags --libs gtk+-3.0) -I../src/worddic/ ../src/worddic/dicentry.c -o parse_line

#include <gtk/gtk.h>

#include "../src/worddic/dicentry.h"

int main( int argc, char **argv )
{

  const gchar *line = "凜 [りん] /(adj-t,adv-to) (1) cold/frigid/bracing/(2) dignified/";
  
  //const gchar *line = "チェイサー  /(n) (1) chaser/pursuer/(2) chaser/light drink (often just water) taken after a strong alcoholic drink/";

  //const gchar *line = "強盗返し [がんとうがえし] /(oK) (n) rotating stage machinery, which rolls backwards 90 degrees to reveal the next scene/";
  
  GjitenDicentry* entry = parse_line(line);
  
  g_printf("Jap definition: %s\n", entry->jap_definition);
  g_printf("Jap reading: %s\n", entry->jap_reading);
  g_printf("Definitions: \n");

  GList *d = NULL;
  for(d = entry->definitions;
      d != NULL;
      d = d->next){
    g_printf("%s\n", d->data);
    
  }
  return 1;
}