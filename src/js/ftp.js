var index = 0;
const sdWebData = document.getElementById("sdWebDataID");
const spiner = document.getElementById("spinerID");
const containerFiles = document.getElementById("containerFilesID");
const containerInfo = document.getElementById("containerInfoID");
document.addEventListener("DOMContentLoaded", () => {
  urlEncodedData = new URLSearchParams();
  urlEncodedData.append("index", index);
  fetch("/ftp_get_data", {
    method: "POST",
    headers: {
      "Content-Type": "application/x-www-form-urlencoded",
    },
    body: urlEncodedData.toString(),
  })
    .then((response) => {
      if (!response.ok)
        throw new Error(response.status + " " + response.statusText);
      else return response.text();
    })
    .then((data) => {
      spiner.style.display = "none";
      sdWebData.innerHTML = "";
      const params = new URLSearchParams(data);
      containerFiles.style.display = "block";
      containerInfo.style.display = "block";
      for (const value of params.getAll("fn")) {
        const fileDiv = document.createElement("div");
        fileDiv.className = "m-1";
        fileDiv.innerHTML = `
          <div class="r">
              <div class="cl cl-nw">${value}</div>
              <div class="cl cl-nw">-</div>
              <div class="cl cl-nw">
                <div class="r j-c">
                  <div class="btn btn-ss btn-i" onclick="DownloadFile('${value}')">
                    <i>&#8595;</i>
                  </div>
                </div>
              </div>
            </div>
          `;
        containerFiles.appendChild(fileDiv);
      }
    })
    .catch((error) => {
      spiner.style.display = "none";
      sdWebData.className = "bd bd-dg";
      sdWebData.innerHTML = error;
    });
});

function DownloadFile(filename) {
  urlEncodedData = new URLSearchParams();
  urlEncodedData.append("filename", filename);
  console.log(urlEncodedData.toString());
  fetch("/ftp_get_file", {
    method: "POST",
    headers: {
      "Content-Type": "application/x-www-form-urlencoded",
    },
    body: urlEncodedData.toString(),
  })
    .then((response) => {
      if (!response.ok)
        throw new Error(response.status + " " + response.statusText);
      else return response.blob();
    })
    .then((blob) => {
      const downloadUrl = URL.createObjectURL(blob);
      const a = document.createElement("a");
      a.href = downloadUrl;
      a.download = filename;
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(downloadUrl);
    })
    .catch((error) => {
      spiner.style.display = "none";
      sdWebData.className = "bd bd-dg";
      sdWebData.innerHTML = error;
    });
}
