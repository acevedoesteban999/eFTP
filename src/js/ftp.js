var index = 0;
const sdWebData = document.getElementById("sdWebDataID");
const spiner = document.getElementById("spinerID");
const containerFiles = document.getElementById("containerFilesID");
const containerInfo = document.getElementById("containerInfoID");
const volumeName = document.getElementById("volumeNameID");
const usedSize = document.getElementById("usedSizeID");
const totalSpace = document.getElementById("totalSpaceID");
const containerSpinner = document.getElementById("containerSpinnerID");
const colSize = document.getElementById("colSizeID");

function formatSize(bytes) {
  const units = ["Bytes", "KB", "MB", "GB", "TB"];
  let index = 0;

  let size = bytes;
  while (size >= 1024 && index < units.length - 1) {
    size /= 1024;
    index++;
  }

  return `${size.toFixed(2)} ${units[index]}`;
}

function getFTPData() {
  containerSpinner.style.display = "block";
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
      console.log(data);
      containerSpinner.style.display = "none";
      sdWebData.innerHTML = "";
      containerInfo.style.display = "block";

      const params = new URLSearchParams(data);

      const volume_name = params.get("volume_name");
      const used_bytes = parseInt(params.get("used_bytes"));
      const total_bytes = parseInt(params.get("total_bytes"));

      const bytes_rel = (100 * used_bytes) / total_bytes;
      if (bytes_rel < 50) colSize.classList = "cl bd bd-ss";
      else if (bytes_rel < 80) colSize.classList = "cl bd bd-wg";
      else colSize.classList = "cl bd bd-dg";

      volumeName.innerHTML = volume_name;
      usedSize.innerHTML = formatSize(used_bytes);
      totalSpace.innerHTML = formatSize(total_bytes);

      const filenames = params.getAll("filename");
      const filesizes = params.getAll("filesize");

      filenames.forEach((filename, index) => {
        const filesize = filesizes[index];
        const fileDiv = document.createElement("div");
        fileDiv.className = "m-1";
        fileDiv.innerHTML = `
            <div class="r">
                <div class="cl cl-nw">${filename}</div>
                <div class="cl cl-nw">${filesize} bytes</div>
                <div class="cl cl-nw">
                    <div class="r j-c">
                        <div class="btn btn-ss btn-i" onclick="DownloadFile('${filename}')">
                            <i>&#8595;</i>
                        </div>
                    </div>
                </div>
            </div>
        `;
        containerFiles.appendChild(fileDiv);
      });
    })
    .catch((error) => {
      spiner.style.display = "none";
      sdWebData.className = "bd bd-dg";
      sdWebData.innerHTML = error;
    });
}

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
