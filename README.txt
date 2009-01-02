A fast in-memory tagging database (an example application would be flickr like image tagging)
 using TokyoCabinet as a backing store. The key-to-tags table is read on start up and searches 
are done on it in-memory. Inserts and updates are transactionally saved to TokyoCabinet as they 
are made. There are lots of things that could be done to improve search performance 
(by several orders of magnitude) and reduce memory consumption, but the current implementation 
focuses on simplicity and is good enough for use in desktop apps.

features:
  - small implementation (~245 C statements) 
  - reasonably small in-memory footprint (~36 bytes per record assuming ~3 tags per record)
  - reasonably fast (less than 10ms to search 1 million records on a 2.5Ghz Core Duo)

