## How to build a docker image for either the notebook server or the web app,
and deploy on Azure.

(Assuming you have an Azure subscription and Docker Hub account.)

### Create docker hub repository

go to
https://hub.docker.com
and sign-in.  You should get a list of your repositories, and a ```Create Repository``` button.  Click this, choose a name, and whether you want it to be
Private or Public.

### Build the docker image

From the SHEEP base directory, do:

```
docker build --target sheep_web -t <username>/sheep_web .
```
(or substitute ```sheep_notebook``` for ```sheep_web``` to build the notebook server).


### Upload the image to docker hub

Login to Docker Hub from the command line:
```
docker login --username=<username> --email=<your_email>
```
Get the image ID using
```docker images```
and tag your image:
```
docker tag <imageID> <username>/<repo_name>:v1.0
```
then push it to docker hub:
```
docker push <username>/<repo_name>
```

### Create an Azure webapp

 * Login to the Azure portal https://portal.azure.com
 * On the left hand bar, click "App Services".
 * Click "+ Add" at the top, then choose "Web App", then "Create" on the bottom right.
 * On the "Create" form, choose an app name (e.g. sheeptest1) - this will form the URL for your app, so needs to be unique.  Choose a subscription and Resource Group (creating one if necessary), and for "OS" choose "Docker".  Then click on
 "Configure container".
 * In the "Container Settings" choose "Docker Hub" as the image source, then
 "Public" or "Private" depending on the settings of your docker hub repo.
 (If it is "Private" you will also need to provide your docker hub login details.)
 * Then click "Create" back on the "Web App create" section.  It will take a
 few minutes to deploy, and after that it will be visible from the "App Services" link on the left of the Azure portal.  If you click on the app here you'll
 get the info for it, including the URL, which should be
 https://<webapp_name>/azurewebsites.net
 * Finally, we need to configure the app to use the correct port.  Go to
 the 
 * In practice, even after the Azure portal said it was available and healthy,
 it took a long time to respond when I first tried to access it through my browser.
 