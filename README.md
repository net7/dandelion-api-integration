# Dandelion API integration
Installing the plugin is really simple:

1. Put the plugin inside your module folder on your Drupal Website
2. Enable it
3. Go to https://dandelion.eu and ask for an API app id and access key
4. Set this info on the configuration (you can find them at: <drupal_website_url>/admin/config/datatxt_api_integration/app_credentials)

# Dandelion classification

1. Create a taxonomy and choose as vocabulary type "DataTXT classification"
2. Add terms to this taxonomy, and for each one, choose one of the wikipedia page that represents it. You can also search for a term and let the module find the link for you.
3. Send the vocabulary to Dandelion with the "Send to DataTXT" button
4. Add this taxonomy to the content type you want to tag with Dandelion API
5. From now on on this content type you will have the "find related concepts" button, that let you automatically tag the content with Dandelion concepts based on your vocabulary 
6. Enjoy!

# Dandelion nex

1. Create a taxonomy and choose as vocabulary type "DataTXT entity extraction"
2. Add this taxonomy to the content type you want to tag with Dandelion matched entities
3. From now on on this content type you will have the "find related concepts" button, that let you automatically tag the content with Dandelion concepts based on your vocabulary 
4. Enjoy!

