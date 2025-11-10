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
  OK: { code: 0, message: "Operation completed successfully" },
  FINISHED: { code: 1, message: "Process finished" },
  OK_NEEDS_REORGANIZING: { code: 2, message: "Success, but bins need reorganizing. Do you want reorganize all items ?" },
  ERROR_OUTPUT_EMPTY: { code: 3, message: "Output bin is empty. Check and try again" },
  ERROR_OUTPUT_NOT_EMPTY: { code: 4, message: "Output bin is not empty. Do you want store the current item and retrieve again ?" },
  ERROR_BIN_NOT_FOUND: { code: 5, message: "Bin not found." },
  ERROR_FULL: { code: 6, message: "Storage is full" },
  ERROR_CAM: { code: 7, message: "Camera error" },
  ERROR_MIXED_ITEM: { code: 8, message: "Mixed items detected. Check items and try again" },
});

function showSuccess(msg) {
  toast.add({ severity: 'success', summary: 'Sucesso', detail: msg, life: 3000 })
}
function showError(msg) {
  toast.add({ severity: 'danger', summary: 'Erro', detail: msg, life: 8000 })
}
function showInfo(msg) {
  toast.add({ severity: 'info', summary: 'Info', detail: msg, life: 8000 })
}

function processStatus(status){
  switch (status) {
    case SpartsOpCode.OK:
      showSuccess(SpartsOpCode.OK.message)
      dialogInfo.value=false
      askReorganize.value = false
      askStoreAndRetrieve.value = false
      break;
    case SpartsOpCode.FINISHED:
      showSuccess(SpartsOpCode.FINISHED.message)
      dialogInfo.value=false
      askReorganize.value = false
      askStoreAndRetrieve.value = false
      break;
    case SpartsOpCode.ERROR_BIN_NOT_FOUND:
      showError(SpartsOpCode.ERROR_BIN_NOT_FOUND.message)
      dialogInfo.value=true
      message.value = SpartsOpCode.ERROR_BIN_NOT_FOUND.message
      askReorganize.value = false
      askStoreAndRetrieve.value = false
      break;
    case SpartsOpCode.ERROR_CAM:
      showError(SpartsOpCode.ERROR_CAM.message)
      dialogInfo.value=true
      message.value = SpartsOpCode.ERROR_CAM.message
      askReorganize.value = false
      askStoreAndRetrieve.value = false
      break;
    case SpartsOpCode.ERROR_FULL:
      showError(SpartsOpCode.ERROR_FULL.message)
      dialogInfo.value=true
      message.value = SpartsOpCode.ERROR_FULL.message
      askReorganize.value = false
      askStoreAndRetrieve.value = false
      break;
    case SpartsOpCode.ERROR_MIXED_ITEM:
      showError(SpartsOpCode.ERROR_MIXED_ITEM.message)
      dialogInfo.value=true
      message.value = SpartsOpCode.ERROR_MIXED_ITEM.message
      askReorganize.value = false
      askStoreAndRetrieve.value = false
      break;
    case SpartsOpCode.ERROR_OUTPUT_EMPTY:
      showError(SpartsOpCode.ERROR_OUTPUT_EMPTY.message)
      dialogInfo.value=true
      message.value = SpartsOpCode.ERROR_OUTPUT_EMPTY.message
      askReorganize.value = false
      askStoreAndRetrieve.value = false
      break;
    case SpartsOpCode.ERROR_OUTPUT_NOT_EMPTY:
      showError(SpartsOpCode.ERROR_OUTPUT_NOT_EMPTY.message)
      dialogInfo.value=true
      message.value = SpartsOpCode.ERROR_OUTPUT_NOT_EMPTY.message
      askReorganize.value = false
      askStoreAndRetrieve.value = true
      break;
    case SpartsOpCode.OK_NEEDS_REORGANIZING:
      showError(SpartsOpCode.OK_NEEDS_REORGANIZING.message)
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
    message.value = `Setup finished`;
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
    dialogInfo.value = false;
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
    message.value = `Map finished`;
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
    dialogInfo.value = false;
  }
}

async function organize(reweight) {
  busy.value = true;
  dialogInfo.value = true;
  message.value = 'Organizing bins';
  showInfo("Executing Organize")
  try {
    const res = await sparts.organize(reweight); // => resolve com {ok: true, status, itemName}
    currentStatus.value = res.status
    processStatus(currentStatus.value)
    getBins()
    message.value = `Organize finished`;
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
    dialogInfo.value = false;
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
    message.value = `Autostore finished`;
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
    dialogInfo.value = false;
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
    console.log("res: "+ res.value)
    message.value = 'Item identified'
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
    dialogInfo.value = false;
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
    message.value = `GetBins concluido`;
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
    message.value = `Store finished`;
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
    dialogInfo.value = false;
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
    message.value = `Retrieving bin finished`;
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
    dialogInfo.value = false;
  }
}

async function storeAndRetrieve(rfidText) {
  busy.value = true;
  dialogInfo.value = true;
  message.value = 'Storing and retrieving bin';
  showInfo("Storing and retrieving bin")
  try {
    const res = await sparts.store(); 
    res = await sparts.retrieve(rfidText); // => resolve com {ok: true, status, itemName}
    currentStatus.value = res.status
    processStatus(currentStatus.value)
    message.value = `Store and retrieving finished`;
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
    dialogInfo.value = false;
  }
}

async function read(id) {
  busy.value = true;
  dialogInfo.value = true;
  message.value = 'Reading slot';
  showInfo("Reading Slot")
  try {
    const res = await sparts.read(id); // => resolve com {ok: true, status, itemName}
    currentStatus.value = res.status
    getBins()
    processStatus(currentStatus.value)
    bucketData.value = bins.value[id].item_name
    message.value = `Read finished`;
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
    dialogInfo.value = false;
  }
}

async function searchItemHandler(){
  dialogSearch.value = true
  getBins()
}


</script>

<template>
  <div>
    <Toast/>
    <Panel class="mb-3">
      <div class="grid justify-content-center">
          <Button class="col-8" label="Connect to SPARTs" v-model:disabled="connected" @click="setup" />
      </div>
      <div class="grid justify-content-center mt-3">
          <Button class="col-8" label="Search item" @click="searchItemHandler" :disabled="!connected || busy"/>
      </div>
      <div class="grid justify-content-center mt-3">
          <Button class="col-8" label="Remap" :disabled="!connected || busy" @click="map"/>
      </div>
      <div class="grid justify-content-center mt-3">
          <Button class="col-8" label="Automatic Storage" :disabled="!connected || busy" @click="autoStore"/>
      </div>
    </Panel>
    
    <Panel header="Reorganize" class="mb-3">
       <div class="grid justify-content-center align-items-center mt-3">
        <div class="flex flex-column align-items-center mt-3 col-2">
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
    <Panel header="Store Bin" class="mb-3">
      <div class="grid justify-content-center align-items-center mt-3">
        <div class="flex flex-column align-items-center mt-3 col-2">
          <span class="mb-2">Change Storage Item ?</span>
          <ToggleSwitch
            v-model="changeStoreItem"
            :disabled="!connected || busy"
          />
        </div>

        <Select
          class="col-2 mr-3"
          v-model="newStorageItem"
          :disabled="!changeStoreItem"
          :options="bins"
          optionLabel="item_name"
          optionValue="item_name"
          size="small"
          placeholder="Select New Item To This Bin"
        />

        <Button
          class="col-4"
          label="Store bin"
          :disabled="!connected || busy"
          @click="store(changeStoreItem, newStorageItem)"
        />
      </div>
    </Panel>
    <Panel header="Read Slot" class="mb-3">
      <div  class="grid justify-content-center align-items-center mt-3">
        <InputText
          class="col-1 mr-3"
          v-model.number="readBucket"
          placeholder="Bin to Read"
          :disabled="!connected || busy"
          
        />
        <Button
          class="col-1"
          label="Read Slot"
          :disabled="!connected || busy"
          @click="read(readBucket)"
        />
        
      </div>
      <div class="grid justify-content-center align-items-center mt-3">
        <Span>Slot Content: {{ bucketData }} </Span>
      </div>
    </Panel>

    

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

    <Dialog v-model:visible="dialogInfo">
      <div class="flex flex-column align-items-center justify-content-center p-4">
        <i class="pi pi-spin pi-spinner" style="font-size: 2rem"></i>
        <span class="mt-3">{{ message }}</span>
        <Button v-if="askReorganize" label="Reorganize" @click="organize" />
        <Button v-if="askStoreAndRetrieve" label="Store and Retrieve" @click="storeAndRetrieve(lastRfid)" />
      </div>
    </Dialog>

  </div>
</template>

<style scoped></style>
