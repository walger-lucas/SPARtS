# SPARtS
Project for the Integration Workshop 3 course on UTFPR-CT.

https://sparts.notion.site/SPARtS-Small-Parts-Automated-Retrieval-System-25a6df57fb9480f09476ff1b71c9ef29?pvs=74

## SPARtS’ Goals

Storing and organizing small parts in workshops is clearly a relevant challenge when considering that finding the correct placement of a part among several possible places can be a laborious task. Usually, extra work is necessary to catalog the placement of each part and train people to know them and use correctly without mixing different items. All that work, however, takes a lot of workable hours and is still very much prone to human error. 

In order to improve the managing of storage and retrieval procedures of a storage of small parts, SPARtS is an Automated Retrieval Storage System in reduced scale that offers: retrieving an item’s storage bin with text search or visual identification, storing several items automatically with visual identification, visualizing the current stock of parts and, with all that, still allows direct manual acess to the bins remaning robust to incorrect placements of storage bins — but be careful, we do not detect if you manually put the wrong items in a storage bin!

## How to Connect To and Use SPARtS

### Install Dependencies
Before you run for the first time, you need to install SPARtS Application dependencies. You can do this by acessing the Linux terminal and running:
```bash
./install_dependencies.sh
```

### Run the Application
1. Run the full Application with the following command:
```bash
./app.sh
```
2. Acess the URL 'http://localhost:5173/' in your Web Browser
3. Turn SPARtS on by its On/Off button
4. Connect to SPARtS Wifi with password 'sparts1234'
5. Click _Connect to SPARtS_ button

Now you can acess all SPARtS functionalities!