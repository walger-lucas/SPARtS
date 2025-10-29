<script setup>
import { Button, Column, InputText, ToggleSwitch, Select, SelectButton, Panel } from 'primevue';
import { ref } from 'vue';
import ApiSpartsClient from '@/services/apiSparts';
const connected = ref (false);
const dialogSearch = ref (false);
const changeStoreItem = ref (false);
const newStorageItem = ref (null);
const readBucket = ref (0);
const busy = ref(false);
const message = ref('');
const imageUrl = ref ('http://127.0.0.1:9000/image')
const bins = ref ([]);
const bucketData = ref(null)


const sparts = new ApiSpartsClient('http://192.168.4.1'); // ou '/api/sparts' se usar proxy

const filters = ref({
  global: { value: null, matchMode: "contains" },
  name: { value: null, matchMode: "contains" },
});

// Exemplo de dados
const products = ref([
  { name: "Laptop", image: "bamboo-watch.jpg", quantity: 5 },
  { name: "Keyboard", image: "black-watch.jpg", quantity: 8 },
  { name: "Mouse", image: "blue-t-shirt.jpg", quantity: 3 },
]);

const SpartsOpCode = Object.freeze({
  OK: 0,
  FINISHED: 1,
  OK_NEEDS_REORGANIZING: 2,
  ERROR_OUTPUT_EMPTY : 3,
  ERROR_OUTPUT_NOT_EMPTY : 4,
  ERROR_BIN_NOT_FOUND : 5,
  ERROR_FULL : 6,
  ERROR_CAM : 7,
  ERROR_MIXED_ITEM : 8,
});

async function setup() {
  busy.value = true;
  message.value = 'Enviando setup...';
  try {
    const res = await sparts.setup(imageUrl.value); // => resolve com {ok: true, status, itemName}
    connected.value = true;
    getBins();
    message.value = `Setup concluido`;
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
  }
}

async function map() {
  busy.value = true;
  message.value = 'Fazendo map';
  try {
    const res = await sparts.map(); // => resolve com {ok: true, status, itemName}
    message.value = `Map concluido`;
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
  }
}

async function organize() {
  busy.value = true;
  message.value = 'Fazendo map';
  try {
    const res = await sparts.organize(); // => resolve com {ok: true, status, itemName}
    message.value = `Organize concluido`;
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
  }
}

async function autoStore() {
  busy.value = true;
  message.value = 'Fazendo autostore';
  try {
    const res = await sparts.autoStore(); // => resolve com {ok: true, status, itemName}
    message.value = `Autostore concluido`;
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
  }
} 

async function image() {
  busy.value = true;
  message.value = 'Capturando imagem';
  try {
    const res = await sparts.image(); // => resolve com {ok: true, status, itemName}
    message.value = `Imagem capturada`;
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
  }
}

async function getBins() {
  busy.value = true;
  message.value = 'Fazendo getBins';
  try {
    const res = await sparts.getBins(); // => resolve com {ok: true, status, itemName}
    bins.value = res.bins;
    message.value = `GetBins concluido`;
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
  }
}

async function store(changeType, newItem) {
  busy.value = true;
  message.value = 'Fazendo store';
  try {
    const res = await sparts.store(changeType, newItem); // => resolve com {ok: true, status, itemName}
    message.value = `Store concluido`;
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
  }
}

async function retrieve(rfidText) {
  busy.value = true;
  message.value = 'Fazendo retrieve';
  try {
    const res = await sparts.retrieve(rfidText); // => resolve com {ok: true, status, itemName}
    message.value = `Retrieve concluido`;
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
  }
}

async function read(id) {
  busy.value = true;
  message.value = 'Fazendo read';
  
  try {
    const res = await sparts.read(id); // => resolve com {ok: true, status, itemName}
    message.value = `Read concluido`;
  } catch (err) {
    message.value = `Erro: ${err?.message ?? String(err)}`;
  } finally {
    busy.value = false;
  }
}


</script>

<template>
  <div>
    {{ bins }}
    <Panel class="mb-3">
      <div class="grid justify-content-center">
          <Button class="col-8" label="Connect to SPARTs" v-model:disabled="connected" @click="setup" />
      </div>
      <div class="grid justify-content-center mt-3">
          <Button class="col-8" label="Search item" @click="dialogSearch = true" :disabled="!connected"/>
      </div>
      <div class="grid justify-content-center mt-3">
          <Button class="col-8" label="Remap" :disabled="!connected" @click="map"/>
      </div>
      <div class="grid justify-content-center mt-3">
          <Button class="col-8" label="Automatic Storage" :disabled="!connected" @click="autoStore"/>
      </div>
      <div class="grid justify-content-center mt-3">
          <Button class="col-8" label="Reorganize" :disabled="!connected" @click="organize"/>
      </div>
    </Panel>
    <Panel header="Store Bin" class="mb-3">
      <div class="grid justify-content-center align-items-center mt-3">
        <div class="flex flex-column align-items-center mt-3 col-2">
          <span class="mb-2">Change Storage Item ?</span>
          <ToggleSwitch
            v-model="changeStoreItem"
          />
        </div>

        <Select
          class="col-2 mr-3"
          v-model="newStorageItem"
          :disabled="!changeStoreItem"
          :options="products"
          size="small"
          placeholder="Select New Item To This Bin"
        />

        <Button
          class="col-4"
          label="Store bin"
          :disabled="!connected"
          @click="store(changeStoreItem, newStorageItem)"
        />
      </div>
    </Panel>
    <Panel header="Read Bucket" class="mb-3">
      <div  class="grid justify-content-center align-items-center mt-3">
        <InputText
          class="col-1 mr-3"
          v-model.number="readBucket"
          placeholder="Bin to Read"
          :disabled="!connected"
          
        />
        <Button
          class="col-1"
          label="Read Bucket"
          :disabled="!connected"
          @click="read(readBucket)"
        />
        
      </div>
      <div class="grid justify-content-center align-items-center mt-3">
        <Span>Bucket Content: </Span>
      </div>
    </Panel>

    <Dialog v-model:visible="dialogSearch" modal header="Search Item" :style="{ width: '50rem' }">
      <div>
        <DataTable :value="products" :filters="filters" filterDisplay="menu" tableStyle="min-width: 50rem">

          <template #header>
          <div class="flex flex-wrap items-center justify-between gap-2">
            <span class="text-xl font-bold">Items</span>
            <span class="p-input-icon-left">
              <InputText placeholder="Search by name" v-model="filters.name.value"/>
            </span>
          </div>
          </template>

          <Column header="Action">
            <template #body="slotProps">
              <Button icon="pi pi-search" class="p-button-rounded" label="Fetch item" />
            </template>
          </Column>

          <Column field="name" header="Name"></Column>

          <Column header="Image">
            <template #body="slotProps">
              <img :src="`https://primefaces.org/cdn/primevue/images/product/${slotProps.data.image}`" :alt="slotProps.data.image" class="w-24 rounded" />
            </template>
          </Column>

          <Column field="quantity" header="Quantity"></Column>

        </DataTable>
      </div>
    </Dialog>
  </div>
</template>

<style scoped></style>
