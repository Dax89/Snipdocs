function getYouTubeFeed() {
  for (let arrScripts = document.getElementsByTagName('script'), i = 0; i < arrScripts.length; i++) {
    if (arrScripts[i].textContent.indexOf('externalId') != -1) {
        let channelId = arrScripts[i].textContent.match(/\"externalId\"\s*\:\s*\"(.*?)\"/)[1];
        let channelRss = 'https://www.youtube.com/feeds/videos.xml?channel_id=' + channelId;
        let channelTitle = document.title.match(/\(?\d*\)?\s?(.*?)\s\-\sYouTube/)[1];
        console.log('The rss feed of the channel \'' + channelTitle + '\' is:\n' + channelRss);
        break;
    }
  }
}
