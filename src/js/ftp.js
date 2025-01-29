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
  s(contentError);
  contentError.innerHTML = error;
  h(formIndex);
  h(btnGetFileData);
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
  s(containerSpinner);
  let f = new FormData(formIndex),
    u = new URLSearchParams(f);
  u.set("index", parseInt(u.get("index")) - 1);
  btnGetFileData.disabled = !0;
  s(containerSpinner);
  h(containerInfo);
  h(containerMessage);
  containerFiles.innerHTML = "";

  fetch("/ftp_get_data", {
    method: "POST",
    headers: {
      "Content-Type": "application/x-www-form-urlencoded",
    },
    body: u.toString(),
  })
    .then((r) => {
      if (!r.ok) throw Error(r.status + " " + r.statusText);
      return r.text();
    })
    .then((d) => {
      h(containerSpinner);
      containerMessage.innerHTML = "";
      s(containerInfo);

      const params = new URLSearchParams(d);

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
        const row = document.createElement("tr");
        row.innerHTML = `
          <td class="t-c">${filename}</td>
          <td class="t-c">${formatSize(filesize)}</td>
          <td>
            <div class="r j-a">
              <div class="btn btn-ss sh btn-i" onclick="DownloadFile('${filename}')">
                <svg width="20" height="20" viewBox="0 0 24 24" fill="none">
                  <path opacity="0.5" fill-rule="evenodd" clip-rule="evenodd" d="M3 14.25C3.41421 14.25 3.75 14.5858 3.75 15C3.75 16.4354 3.75159 17.4365 3.85315 18.1919C3.9518 18.9257 4.13225 19.3142 4.40901 19.591C4.68577 19.8678 5.07435 20.0482 5.80812 20.1469C6.56347 20.2484 7.56459 20.25 9 20.25H15C16.4354 20.25 17.4365 20.2484 18.1919 20.1469C18.9257 20.0482 19.3142 19.8678 19.591 19.591C19.8678 19.3142 20.0482 18.9257 20.1469 18.1919C20.2484 17.4365 20.25 16.4354 20.25 15C20.25 14.5858 20.5858 14.25 21 14.25C21.4142 14.25 21.75 14.5858 21.75 15V15.0549C21.75 16.4225 21.75 17.5248 21.6335 18.3918C21.5125 19.2919 21.2536 20.0497 20.6517 20.6516C20.0497 21.2536 19.2919 21.5125 18.3918 21.6335C17.5248 21.75 16.4225 21.75 15.0549 21.75H8.94513C7.57754 21.75 6.47522 21.75 5.60825 21.6335C4.70814 21.5125 3.95027 21.2536 3.34835 20.6517C2.74643 20.0497 2.48754 19.2919 2.36652 18.3918C2.24996 17.5248 2.24998 16.4225 2.25 15.0549C2.25 15.0366 2.25 15.0183 2.25 15C2.25 14.5858 2.58579 14.25 3 14.25Z" fill="currentColor"/>
                  <path fill-rule="evenodd" clip-rule="evenodd" d="M12 16.75C12.2106 16.75 12.4114 16.6615 12.5535 16.5061L16.5535 12.1311C16.833 11.8254 16.8118 11.351 16.5061 11.0715C16.2004 10.792 15.726 10.8132 15.4465 11.1189L12.75 14.0682V3C12.75 2.58579 12.4142 2.25 12 2.25C11.5858 2.25 11.25 2.58579 11.25 3V14.0682L8.55353 11.1189C8.27403 10.8132 7.79963 10.792 7.49393 11.0715C7.18823 11.351 7.16698 11.8254 7.44648 12.1311L11.4465 16.5061C11.5886 16.6615 11.7894 16.75 12 16.75Z" fill="currentColor"/>
                </svg>
              
              </div>
              <div class="btn btn-dg sh btn-i" onclick="ShowModalDeleteFile('${filename}')">
                <svg width="20" height="20" viewBox="0 0 24 24" fill="none">
                  <path d="M4 7H20" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
                  <path d="M6 10L7.70141 19.3578C7.87432 20.3088 8.70258 21 9.66915 21H14.3308C15.2974 21 16.1257 20.3087 16.2986 19.3578L18 10" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
                  <path d="M9 5C9 3.89543 9.89543 3 11 3H13C14.1046 3 15 3.89543 15 5V7H9V5Z" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
                </svg>
              </div>
            </div>
          </td>
        `;
        containerFiles.appendChild(row);
      });

      btnGetFileData.disabled = !1;
    })
    .catch((e) => {
      h(containerSpinner);
      s(containerMessage);
      containerMessage.className = "bd bd-dg";
      containerMessage.innerHTML = e;
      btnGetFileData.disabled = !1;
    });
}

function ShowModalDeleteFile(filename) {
  sM("h3 bd bd-dg my-3 p-2", "Delete file: " + filename + " ?");
  const btns = document.createElement("div");
  document.getElementById("mrbtnid").innerHTML = `
      <button class="btn btn-sc" onclick="hM()">
        Close
      </button>
      <button id="mbtndid" class="btn btn-dg" onclick="DeleteFile('${filename}')">
        Delete file
      </button>  
      `;
}

function DownloadFile(file) {
  urlEncodedData = new URLSearchParams();
  urlEncodedData.append("filename", file);
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
      a.download = file;
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(downloadUrl);
    })
    .catch((e) => {
      h(containerSpinner);
      s(containerMessage);
      containerMessage.className = "bd bd-dg";
      containerMessage.innerHTML = e;
    });
}

function DeleteFile(file) {
  urlEncodedData = new URLSearchParams();
  urlEncodedData.append("filename", file);
  fetch("/ftp_delete_file", {
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
    .then((data) => {
      hM();
      getFTPData();
    })
    .catch((e) => {
      h(document.getElementById("mbtndid"));
      sM("h3 bd bd-dg my-3 p-2", e);
    });
}
