const containerFiles = document.getElementById("containerFilesID");
const containerInfo = document.getElementById("containerInfoID");
const volumeName = document.getElementById("volumeNameID");
const usedSize = document.getElementById("usedSizeID");
const totalSpace = document.getElementById("totalSpaceID");
const containerSpinner = document.getElementById("containerSpinnerID");
const colSize = document.getElementById("colSizeID");
const formIndex = document.getElementById("formIndexID");
const btnGetFileData = document.getElementById("btnGetFileDataID");
const containerMessage = document.getElementById("containerMessageID");
const contentError = document.getElementById("contentErrorId");

function setError(error) {
  contentError.style.display = "block";
  contentError.innerHTML = error;
  formIndex.style.display = "none";
  btnGetFileData.style.display = "none";
}

function formatSize(bytes) {
  const units = ["Bytes", "KB", "MB", "GB", "TB"];
  let index = 0;

  let size = parseInt(bytes);
  while (size >= 1024 && index < units.length - 1) {
    size /= 1024;
    index++;
  }

  return `${size.toFixed(2)} ${units[index]}`;
}

function getFTPData() {
  if (!formIndex.checkValidity()) {
    formIndex.reportValidity();
    return;
  }
  containerSpinner.style.display = "block";
  formData = new FormData(formIndex);
  urlsearchparams = new URLSearchParams(formData);
  urlsearchparams.set("index", parseInt(urlsearchparams.get("index")) - 1);
  btnGetFileData.disabled = true;

  containerSpinner.style.display = "block";
  containerInfo.style.display = "none";
  containerMessage.style.display = "none";
  containerFiles.innerHTML = "";

  fetch("/ftp_get_data", {
    method: "POST",
    headers: {
      "Content-Type": "application/x-www-form-urlencoded",
    },
    body: urlsearchparams.toString(),
  })
    .then((response) => {
      if (!response.ok)
        throw new Error(response.status + " " + response.statusText);
      else return response.text();
    })
    .then((data) => {
      containerSpinner.style.display = "none";
      containerMessage.innerHTML = "";
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
                <div class="cl cl-nw">${formatSize(filesize)}</div>
                <div class="cl cl-nw">
                    <div class="r j-c">
                        <div class="btn btn-sc sh btn-i" onclick="DownloadFile('${filename}')">
                          <svg width="30" height="30" fill="currentColor" viewBox="0 0 16 16">
                            <path fill-rule="evenodd" d="M8.5 4.5a.5.5 0 0 0-1 0v5.793L5.354 8.146a.5.5 0 1 0-.708.708l3 3a.5.5 0 0 0 .708 0l3-3a.5.5 0 0 0-.708-.708L8.5 10.293z"/>
                          </svg>
                        </div>
                    </div>
                </div>
            </div>
        `;
        containerFiles.appendChild(fileDiv);
      });

      btnGetFileData.disabled = false;
    })
    .catch((error) => {
      containerSpinner.style.display = "none";
      containerMessage.style.display = "block";
      containerMessage.className = "bd bd-dg";
      containerMessage.innerHTML = error;
      btnGetFileData.disabled = false;
    });
}

function DownloadFile(filename) {
  urlEncodedData = new URLSearchParams();
  urlEncodedData.append("filename", filename);
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
      containerSpinner.style.display = "none";
      containerMessage.className = "bd bd-dg";
      containerMessage.innerHTML = error;
    });
}
