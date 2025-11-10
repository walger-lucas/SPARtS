<script setup>
import { Button, Column, InputText, ToggleSwitch, Select, SelectButton, Panel, useToast, Toast, Dialog } from 'primevue';
import { ref } from 'vue';
import ApiSpartsClient from '@/services/apiSparts';
const connected = ref (false);
const dialogSearch = ref (false);
const dialogInfo = ref(false)
const changeStoreItem = ref (false);
const newStorageItem = ref (null);
const readBucket = ref (0);
const busy = ref(false);
const message = ref('');
const imageUrl = ref ('http://192.168.4.2:9000/image')
const bins = ref ([]);
const reweight = ref(false)
const lastItem = ref('')
const currentStatus = ref(null)
const bucketData = ref(null)
const toast = useToast()
const askReorganize = ref (false)
const askStoreAndRetrieve = ref (false)
const lastRfid = ref (null)



const sparts = new ApiSpartsClient('http://192.168.4.1'); // ou '/api/sparts' se usar proxy

const filters = ref({
  global: { value: null, matchMode: "contains" },
  item_name: { value: null, matchMode: "contains" },
});

const SpartsOpCode = Object.freeze({
  OK: { code: "OK", message: "Operation completed successfully" },
  FINISHED: { code: "FINISHED", message: "Process finished" },
  OK_NEEDS_REORGANIZING: { code: "OK_NEEDS_REORGANIZING", message: "Success, but bins need reorganizing. Do you want reorganize all items ?" },
  ERROR_OUTPUT_EMPTY: { code: "ERROR_OUTPUT_EMPTY", message: "Output bin is empty. Check and try again" },
  ERROR_OUTPUT_NOT_EMPTY: { code: "ERROR_OUTPUT_NOT_EMPTY", message: "Output bin is not empty. Do you want store the current item and retrieve again ?" },
  ERROR_BIN_NOT_FOUND: { code: "ERROR_BIN_NOT_FOUND", message: "Bin not found." },
  ERROR_FULL: { code: "ERROR_FULL", message: "Storage is full" },
  ERROR_CAM: { code: "ERROR_CAM", message: "Camera error" },
  ERROR_MIXED_ITEM: { code: "ERROR_MIXED_ITEM", message: "Mixed items detected. Check items and try again" },
});

function showSuccess(msg) {
  toast.add({ severity: 'success', summary: 'Sucesso', detail: msg, life: 3000 })
}
function showError(msg) {
  toast.add({ severity: 'error', summary: 'Erro', detail: msg, life: 8000 })
}
function showInfo(msg) {
  toast.add({ severity: 'info', summary: 'Info', detail: msg, life: 8000 })
}

function processStatus(status){
  switch (status) {
    case SpartsOpCode.OK.code:
      showSuccess(SpartsOpCode.OK.message)
      message.value = SpartsOpCode.OK.message
      dialogInfo.value=true
      askReorganize.value = false
      askStoreAndRetrieve.value = false
      break;
    case SpartsOpCode.FINISHED.code:
      showSuccess(SpartsOpCode.FINISHED.message)
      message.value = SpartsOpCode.FINISHED.message
      dialogInfo.value=true
      askReorganize.value = false
      askStoreAndRetrieve.value = false
      break;
    case SpartsOpCode.ERROR_BIN_NOT_FOUND.code:
      showError(SpartsOpCode.ERROR_BIN_NOT_FOUND.message)
      dialogInfo.value=true
      message.value = SpartsOpCode.ERROR_BIN_NOT_FOUND.message
      askReorganize.value = false
      askStoreAndRetrieve.value = false
      break;
    case SpartsOpCode.ERROR_CAM.code:
      showError(SpartsOpCode.ERROR_CAM.message)
      dialogInfo.value=true
      message.value = SpartsOpCode.ERROR_CAM.message
      askReorganize.value = false
      askStoreAndRetrieve.value = false
      break;
    case SpartsOpCode.ERROR_FULL.code:
      showError(SpartsOpCode.ERROR_FULL.message)
      dialogInfo.value=true
      message.value = SpartsOpCode.ERROR_FULL.message
      askReorganize.value = false
      askStoreAndRetrieve.value = false
      break;
    case SpartsOpCode.ERROR_MIXED_ITEM.code:
      showError(SpartsOpCode.ERROR_MIXED_ITEM.message)
      dialogInfo.value=true
      message.value = SpartsOpCode.ERROR_MIXED_ITEM.message
      askReorganize.value = false
      askStoreAndRetrieve.value = false
      break;
    case SpartsOpCode.ERROR_OUTPUT_EMPTY.code:
      showError(SpartsOpCode.ERROR_OUTPUT_EMPTY.message)
      dialogInfo.value=true
      message.value = SpartsOpCode.ERROR_OUTPUT_EMPTY.message
      askReorganize.value = false
      askStoreAndRetrieve.value = false
      break;
    case SpartsOpCode.ERROR_OUTPUT_NOT_EMPTY.code:
      showError(SpartsOpCode.ERROR_OUTPUT_NOT_EMPTY.message)
      dialogInfo.value=true
      message.value = SpartsOpCode.ERROR_OUTPUT_NOT_EMPTY.message
      askReorganize.value = false
      askStoreAndRetrieve.value = true
      break;
    case SpartsOpCode.OK_NEEDS_REORGANIZING.code:
      showInfo(SpartsOpCode.OK_NEEDS_REORGANIZING.message)
      dialogInfo.value=true
      message.value = SpartsOpCode.OK_NEEDS_REORGANIZING.message
      askReorganize.value = true
      askStoreAndRetrieve.value = false
      break;
    default:
      showInfo(SpartsOpCode.OK.message)
      dialogInfo.value=false
      message.value = ""
      askReorganize.value = false
      askStoreAndRetrieve.value = false
      break;
  }

}

async function setup() {
  busy.value = true;
  dialogInfo.value = true;
  message.value = 'Doing Setup';
  showInfo("Executing Setup !")
  try {
    const res = await sparts.setup(imageUrl.value); // => resolve com {ok: true, status, itemName}
    connected.value = true;
    currentStatus.value = res.status
    processStatus(currentStatus.value)
    getBins();
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
    
  }
}

async function map() {
  busy.value = true;
  dialogInfo.value = true;
  message.value = 'Mapping bins';
  showInfo("Executing Map")
  try {
    const res = await sparts.map(); // => resolve com {ok: true, status, itemName}
    currentStatus.value = res.status
    processStatus(currentStatus.value)
    getBins();
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
  }
}

async function organize(reweight) {
  busy.value = true;
  dialogInfo.value = true;
  message.value = 'Organizing bins';
  showInfo("Executing Organize")
  try {
    askReorganize.value = false
    const res = await sparts.organize(reweight); // => resolve com {ok: true, status, itemName}
    currentStatus.value = res.status
    processStatus(currentStatus.value)
    getBins()
    message.value = `Organize finished`;
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
  }
}

async function autoStore() {
  busy.value = true;
  dialogInfo.value = true;
  message.value = 'Executing autostore';
  showInfo("Executing Store")
  try {
    const res = await sparts.autoStore(); // => resolve com {ok: true, status, itemName}
    currentStatus.value = res.status
    processStatus(currentStatus.value)
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
  }
} 

async function image() {
  busy.value = true;
  dialogInfo.value = true;
  message.value = 'Identifying Item';
  showInfo("Taking Picure")
  try {
    const res = await sparts.image(); // => resolve com {ok: true, status, itemName}
    lastItem.value = res.itemName;
    currentStatus.value = res.status
    processStatus(currentStatus.value)
    filters.value.item_name.value = lastItem.value;
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
  }
}

async function getBins() {
  // busy.value = true;
  // message.value = 'Fazendo getBins';
  // showInfo("Getting Bins")
  try {
    const res = await sparts.getBins(); // => resolve com {ok: true, status, itemName}
    bins.value = res.bins;
    currentStatus.value = res.status
    // processStatus(currentStatus.value)
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    // busy.value = false;
  }
}

async function store(changeType, newItem) {
  busy.value = true;
  dialogInfo.value = true;
  message.value = 'Executing Store';
  showInfo("Executing Store")
  try {
    const res = await sparts.store(changeType, newItem); // => resolve com {ok: true, status, itemName}
    currentStatus.value = res.status
    processStatus(currentStatus.value)
    getBins()
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
  }
}

async function retrieve(rfidText) {
  busy.value = true;
  dialogInfo.value = true;
  lastRfid.value = rfidText
  message.value = 'Retrieving bin';
  showInfo("Executing Retrieve")
  try {
    const res = await sparts.retrieve(rfidText); // => resolve com {ok: true, status, itemName}
    currentStatus.value = res.status
    processStatus(currentStatus.value)
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
  }
}

async function storeAndRetrieve(rfidText) {
  busy.value = true;
  dialogInfo.value = true;
  message.value = 'Storing and retrieving bin';
  showInfo("Storing and retrieving bin")
  try {
    askStoreAndRetrieve.value = false
    await sparts.store(); 
    const res = await sparts.retrieve(rfidText); // => resolve com {ok: true, status, itemName}
    currentStatus.value = res.status
    processStatus(currentStatus.value)
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
  }
}

async function read(id) {
  busy.value = true;
  dialogInfo.value = true;
  message.value = 'Reading slot';
  showInfo("Reading Slot")
  try {
    if (id > 0 && id <25){
      var new_id = id -1
    } else{
      new_id=255
    }
    const res = await sparts.read(new_id); // => resolve com {ok: true, status, itemName}
    currentStatus.value = res.status
    processStatus(currentStatus.value)
    getBins()
    bucketData.value = findBin(new_id)
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
  }
}

function findBin(position) {
  const bin = bins.value.find((item) => item.position === position);
  return bin ? bin.item_name : null;
}

async function searchItemHandler(){
  dialogSearch.value = true
  getBins()
}

async function handlerHide() {
  askReorganize.value = false
  askStoreAndRetrieve.value = false
}


</script>

<template>
  <div>
    <Toast/>
    <div class="flex justify-content-center">
      <img src="/SPARtS.png" class="w-30rem" />
    </div>
    <div class="grid justify-content-center align-items-center mt-3">
      <Button class="col-2" label="Connect to SPARTs" v-model:disabled="connected" @click="setup" />
    </div>
    <div class="grid justify-content-center ml-1 mr-1">
      <Panel class="col-5 mt-3 mr-3" header="Retrieval">
        <div class="grid justify-content-center mt-3">
          <Button class="col-8" label="Search item" @click="searchItemHandler" :disabled="!connected || busy"/>
        </div>
        <Panel header="Get Slot Contend" class="mt-3">
          <div  class="grid justify-content-center align-items-center mt-3">
            <InputText
              class="col-4 mr-1"
              v-model.number="readBucket"
              placeholder="Bin to Read"
              :disabled="!connected || busy"
              
            />
            <Button
              class="col-4"
              label="Read Slot"
              :disabled="!connected || busy"
              @click="read(readBucket)"
            />
          </div>
          <div class="grid justify-content-center align-items-center mt-6">
            <Span>Slot Content: {{ bucketData }} </Span>
          </div>
        </Panel>

      </Panel>
      <Panel class="col-5 mt-3 " header="Storage">
        <div class="grid justify-content-center mt-3">
          <Button class="col-8" label="Automatic Storage" :disabled="!connected || busy" @click="autoStore"/>
        </div>
        <Panel class="mt-3">
          <div class="grid justify-content-center">
            <Button
            class="col-8"
            label="Store bin"
            :disabled="!connected || busy"
            @click="store(changeStoreItem, newStorageItem)"
            />
            <div class="grid justify-content-center align-items-center mt-3">
              <div class="flex flex-column align-items-center mt-3 col-4">
                <span class="mb-2 align-items-center">Change Storage Item ?</span>
                <ToggleSwitch
                  v-model="changeStoreItem"
                  :disabled="!connected || busy"
                />
              </div>

              <Select
                class="col-4 mr-3"
                v-model="newStorageItem"
                :disabled="!changeStoreItem"
                :options="bins"
                optionLabel="item_name"
                optionValue="item_name"
                size="small"
                placeholder="Select New Item To This Bin"
              />
            </div>
          </div>
        </Panel>
      </Panel>
    </div>
    <div class="grid justify-content-center">
      <Panel class="mt-3 col-10" header="Reorganization">
        <div class="grid justify-content-center mt-3">
          <Button class="col-8" label="Remap" :disabled="!connected || busy" @click="map"/>
        </div>

        <div class="grid justify-content-center align-items-center mt-3">
          <div class="flex flex-column align-items-center mt-3 col-4">
            <span class="mb-2">Reweight ?</span>
            <ToggleSwitch
              v-model="reweight"
              :disabled="!connected || busy"
            />
          </div>

          <Button
            class="col-4"
            label="Reorganize"
            :disabled="!connected || busy"
            @click="organize(reweight)"
          />
        </div>
      </Panel>
    </div>  

    <Dialog v-model:visible="dialogSearch" modal header="Search Item" :style="{ width: '50rem' }">
      <div>
        <DataTable :value="bins" :filters="filters" filterDisplay="menu" tableStyle="min-width: 50rem">

          <template #header>
          <div class="flex flex-wrap items-center justify-between gap-2">
            <span class="text-xl font-bold">Items</span>
            <span class="p-input-icon-left">
              <InputText placeholder="Search by name" v-model="filters.item_name.value"/>
            </span>
            <Button label="Find by image" @click="image" />

          </div>
          </template>

          <Column header="Action">
            <template #body="slotProps">
              <Button icon="pi pi-search" class="p-button-rounded" label="Fetch item"  @click="retrieve(slotProps.data.rfid)"/>
            </template>
          </Column>

          <Column field="item_name" header="Name" sortable></Column>

          <Column header="Image">
            <template #body="slotProps">
              <img :src="`/images/${slotProps.data.item_name}.png`" class="w-10 rounded" />
            </template>
          </Column>

          <Column field="amount" header="Quantity"></Column>

          <Column field="unit_weight" header="Unitary weight">
              <template #body="slotProps">
                {{ slotProps.data.unit_weight  }} g
              </template>
          </Column>

        </DataTable>
      </div>
    </Dialog>

    <Dialog v-model:visible="dialogInfo" @hide="handlerHide">
      <div class="flex flex-column align-items-center justify-content-center p-4">
        <i v-if="busy" class="pi pi-spin pi-spinner" style="font-size: 2rem"></i>
        <span class="mt-3">{{ message }} </span>
        <Button v-if="askReorganize" label="Reorganize" @click="organize(false)" />
        <Button v-if="askStoreAndRetrieve" label="Store and Retrieve" @click="storeAndRetrieve(lastRfid)" />
        <Button v-if="askStoreAndRetrieve || askReorganize" label="Cancel" @click="dialogInfo = false" />
      </div>
    </Dialog>

  </div>
</template>

<style scoped>
  .p-panel {
    border-color: #d40606!important;
  }

  .p-panel .p-panel-header {
    font-size: 20px !important;
  }
</style>
